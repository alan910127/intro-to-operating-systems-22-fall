#include <dirent.h>
#include <fcntl.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "tiny_ftp.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

using tiny_ftp::ChangeInfo;
using tiny_ftp::DEntry;
using tiny_ftp::DEntry_PathType;
using tiny_ftp::Directory;
using tiny_ftp::FileChunk;
using tiny_ftp::FileInfo;
using tiny_ftp::FtpServer;
using tiny_ftp::FtpStatus;
using tiny_ftp::Path;
using tiny_ftp::SessionID;
using tiny_ftp::User;

namespace fs = std::filesystem;

struct FileFS {
  std::string name;
  int size;
};

struct DirectoryFS {
  std::string name;
  std::vector<FileFS *> files;
  std::vector<DirectoryFS *> subDirectory;
};

DirectoryFS *root;

std::unordered_map<std::string, std::string> currentDirectory;

DirectoryFS *FindDirectory(DirectoryFS *dir, std::string targetDirName) {
  if (!dir) return nullptr;

  if (dir->name == targetDirName) return dir;

  for (auto iter = dir->subDirectory.begin(); iter != dir->subDirectory.end();
       iter++) {
    if ((*iter)->name == targetDirName) {
      return *iter;
    }
    return FindDirectory(*iter, targetDirName);
  }
  return nullptr;
}

// Logic and data behind the server's behavior.
class FtpServerServiceImpl final : public FtpServer::Service {
  Status Login(ServerContext *context, const User *user,
               SessionID *sessionid) override {
    std::cout << "User " + user->name() + " login with pwd: " + user->pwd()
              << std::endl;
    sessionid->set_id("54321");
    currentDirectory[sessionid->id()] = "/";
    return Status::OK;
  }

  Status Logout(ServerContext *context, const SessionID *sessionid,
                FtpStatus *status) override {
    std::cout << "Logout session: " << sessionid->id() << std::endl;
    status->set_code(0);
    return Status::OK;
  }

  Status ListDirectory(ServerContext *context, const SessionID *sessionid,
                       Directory *directory) override {
    std::cout << "List the directory of the session: " + sessionid->id()
              << std::endl;

    fs::path dir{currentDirectory[sessionid->id()]};

    for (const auto &dirEntry : fs::directory_iterator{dir}) {
      std::string entryName = dirEntry.path();
      if (entryName == "." || entryName == "..") {
        continue;
      }

      DEntry *entry = directory->add_dentries();
      entry->set_name(entryName);

      if (dirEntry.is_directory()) {
        entry->set_type(DEntry_PathType::DEntry_PathType_DIRECTORY);
      } else {
        entry->set_type(DEntry_PathType::DEntry_PathType_FILE);
      }

      entry->set_size(dirEntry.file_size());
    }

    return Status::OK;
  }

  Status GetWorkingDirectory(ServerContext *context, const SessionID *sessionid,
                             Path *path) override {
    std::cout << "Get the working directory of the session: " + sessionid->id()
              << std::endl;
    // Set currentDirectory of sessionid to path

    fs::path workingDir{currentDirectory[sessionid->id()]};

    path->set_path(workingDir.string());

    return Status::OK;
  }

  Status ChangeWorkingDirectory(ServerContext *context,
                                const ChangeInfo *changeinfo,
                                FtpStatus *status) override {
    std::cout << "The working directory of the session: " +
                     changeinfo->sessionid().id() + " has changed to " +
                     changeinfo->path().path()
              << std::endl;
    // Set Path to currentDirectory of sessionid

    // Confirm seccess with opendir, if so, set status to 0, else set status to
    // 1

    return Status::OK;
  }

  Status DownloadSmallFile(ServerContext *context, const ChangeInfo *changeinfo,
                           FileChunk *filechunk) override {
    std::cout << "Download the file on path: " +
                     currentDirectory[changeinfo->sessionid().id()] + '/' +
                     changeinfo->path().path() +
                     " of the session: " + changeinfo->sessionid().id()
              << std::endl;
    // Use open, fstat, read, to set the element of filechunk

    // If download is fail, set size of filechunk to -1

    return Status::OK;
  }

  Status UploadSmallFile(ServerContext *context, const FileInfo *fileinfo,
                         FtpStatus *status) override {
    std::cout << "Start Upload File to: "
              << currentDirectory[fileinfo->changeinfo().sessionid().id()] +
                     '/' + fileinfo->changeinfo().path().path()
              << std::endl;

    // if file size is -1, it means upload fail
    if (fileinfo->filechunk().size() == -1) {
      status->set_code(1);
      return Status::CANCELLED;
    }

    int fd = open((currentDirectory[fileinfo->changeinfo().sessionid().id()] +
                   '/' + fileinfo->changeinfo().path().path())
                      .c_str(),
                  O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
      std::cout << "Open File Succeed" << std::endl;
      if (write(fd, fileinfo->filechunk().data().c_str(),
                fileinfo->filechunk().size()) > 0) {
        status->set_code(0);
        std::cout << "Upload Finish" << std::endl;
        close(fd);
        return Status::OK;
      } else {
        close(fd);
        status->set_code(1);
        return Status::CANCELLED;
      }
    } else {
      status->set_code(1);
      return Status::CANCELLED;
    }
  }
};

void ConstructFileSystem() {
  root = new DirectoryFS();
  root->name = "/";

  DirectoryFS *home;
  home = new DirectoryFS();
  home->name = "home";

  DirectoryFS *Desktop;
  Desktop = new DirectoryFS();
  Desktop->name = "Desktop";

  FileFS *file1, *file2;
  file1 = new FileFS();
  file2 = new FileFS();
  file1->name = "README.md";
  file1->size = 128;
  file2->name = "SOS.txt";
  file2->size = 1024;
  Desktop->files.push_back(file1);
  Desktop->files.push_back(file2);

  home->subDirectory.push_back(Desktop);
  root->subDirectory.push_back(home);
}

void DeconstructFileSystem(DirectoryFS *dir) {
  if (!dir) {
    return;
  }

  for (auto iter = dir->files.begin(); iter != dir->files.end(); iter++) {
    delete *iter;
  }

  for (auto iter = dir->subDirectory.begin(); iter != dir->subDirectory.end();
       iter++) {
    DeconstructFileSystem(*iter);
  }

  delete dir;
}

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  FtpServerServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;

  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);

  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

void signal_handler(int signal) {
  DeconstructFileSystem(root);
  exit(signal);
}

int main(int argc, char **argv) {
  ConstructFileSystem();
  // DirectoryFS *temp = FindDirectory(root, "/");
  // if(temp)
  //     std::cout<<temp->name;
  signal(SIGINT, signal_handler);

  RunServer();
  // DeconstructFileSystem(root);

  return 0;
}