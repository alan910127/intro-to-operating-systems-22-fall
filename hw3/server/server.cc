#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <signal.h>

#include <iostream>
#include <memory>
#include <string>
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
using tiny_ftp::FtpServer;
using tiny_ftp::FtpStatus;
using tiny_ftp::Path;
using tiny_ftp::SessionID;
using tiny_ftp::User;

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

DirectoryFS *currentDirectory;

// This is a recursive function, you should pass *root as dir when you first
// call it.
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

/*
    For OS Student: The class FtpServerServiceImpl is the logic and data behind
   the server's behavior. Your main job is implementing each RPC call function.
   We provide ListDirectory() as a example of constructing a response message
   for the client. ( ListDirectory() is already implemented, You may but don't
   have to modify it.)

    Remember, you need to change the return status in each RPC call to "OK" once
   you implement the function!
*/
class FtpServerServiceImpl final : public FtpServer::Service {
  // The second parameter means user's request message.
  // The third one means the output that server should response.
  Status Login(ServerContext *context, const User *user,
               SessionID *sessionid) override {
    // Parse and log user's request in stdout
    std::string username = user->name();
    std::string password = user->pwd();
    std::cout << "User: " << username << " login with pwd: " << password
              << std::endl;

    // Set fake session id for response
    sessionid->set_id("f@k3_s3$$10n_1d");

    // Set currentDirectory to root since the user just login
    currentDirectory = root;

    return Status::OK;
  }

  Status Logout(ServerContext *context, const SessionID *sessionid,
                FtpStatus *status) override {
    // Parse and log user's request in stdout
    std::string sessionId = sessionid->id();
    std::cout << "Logout session: " << sessionId << std::endl;

    // Set fake status code for response
    status->set_code(0);

    return Status::OK;
  }

  Status ListDirectory(ServerContext *context, const SessionID *sessionid,
                       Directory *directory) override {
    // Parse and log user's request in stdout
    std::string sessionId = sessionid->id();
    std::cout << "List the directory of the session: " + sessionId << std::endl;

    // Construct the response by traversing the pseudo file system
    for (auto iter = currentDirectory->subDirectory.begin();
         iter != currentDirectory->subDirectory.end(); iter++) {
      DEntry *tmp;
      tmp = directory->add_dentries();
      tmp->set_name((*iter)->name);
      tmp->set_type(DEntry_PathType::DEntry_PathType_DIRECTORY);
      tmp->set_size(0);
    }

    for (auto iter = currentDirectory->files.begin();
         iter != currentDirectory->files.end(); iter++) {
      DEntry *tmp;
      tmp = directory->add_dentries();
      tmp->set_name((*iter)->name);
      tmp->set_type(DEntry_PathType::DEntry_PathType_FILE);
      tmp->set_size((*iter)->size);
    }

    return Status::OK;
  }

  Status GetWorkingDirectory(ServerContext *context, const SessionID *sessionid,
                             Path *path) override {
    // Parse and log user's request in stdout
    std::string sessionId = sessionid->id();
    std::cout << "Get the working directory of the session: " << sessionId
              << std::endl;

    // Set response path to the name of currentDirectory
    path->set_path(currentDirectory->name);

    return Status::OK;
  }

  Status ChangeWorkingDirectory(ServerContext *context,
                                const ChangeInfo *changeinfo,
                                FtpStatus *status) override {
    // Parse and log user's request in stdout
    std::string sessionId = changeinfo->sessionid().id();
    std::string path = changeinfo->path().path();

    std::cout << "The working directory of the session: " << sessionId
              << " has changed to " << path << std::endl;

    // Change currentDirectory base on changeinfo by calling FindDirectory
    currentDirectory = FindDirectory(root, path);
    status->set_code(0);

    return Status::OK;
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

  signal(SIGINT, signal_handler);

  RunServer();

  return 0;
}