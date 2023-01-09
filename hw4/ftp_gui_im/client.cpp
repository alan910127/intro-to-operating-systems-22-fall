/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "client.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <vector>

FtpClient::FtpClient(std::shared_ptr<Channel> channel)
    : stub_(FtpServer::NewStub(channel)) {}

std::vector<FtpClient::DirectoryVec> FtpClient::ChangeDirectory(
    std::string directory) {
  std::cout << "Change local directory to: " + directory << std::endl;

  DIR *dir = opendir(directory.c_str());

  struct dirent *ent;

  std::vector<FtpClient::DirectoryVec> listDirectoryRes;

  if (dir) {
    WorkingDirectory = directory;

    while ((ent = readdir(dir)) != NULL) {
      std::string name = ent->d_name;
      if (name == ".." || name == ".") continue;
      FtpClient::DirectoryVec temp;
      temp.name = name;
      if (ent->d_type == 4)
        temp.type = "_DIR";
      else
        temp.type = "_FILE";
      struct stat st = {};
      int fd = open((WorkingDirectory + '/' + name).c_str(), O_RDONLY);
      fstat(fd, &st);
      temp.size = st.st_size;
      listDirectoryRes.push_back(temp);
    }
  }

  return listDirectoryRes;
}

std::vector<FtpClient::DirectoryVec> FtpClient::ListCurrentDirectory() {
  std::cout << "List local directory to: " + WorkingDirectory << std::endl;

  DIR *dir = opendir(WorkingDirectory.c_str());

  struct dirent *ent;

  std::vector<FtpClient::DirectoryVec> listDirectoryRes;

  if (dir) {
    while ((ent = readdir(dir)) != NULL) {
      std::string name = ent->d_name;
      if (name == ".." || name == ".") continue;
      FtpClient::DirectoryVec temp;
      temp.name = name;
      if (ent->d_type == 4)
        temp.type = "_DIR";
      else
        temp.type = "_FILE";
      struct stat st = {};
      int fd = open((WorkingDirectory + '/' + name).c_str(), O_RDONLY);
      fstat(fd, &st);
      temp.size = st.st_size;
      listDirectoryRes.push_back(temp);
    }
  }

  return listDirectoryRes;
}

std::string FtpClient::LoginTest(std::string userName, std::string userPasswd) {
  User user;
  user.set_name(userName);
  user.set_pwd(userPasswd);

  SessionID replySessionID;

  ClientContext context;

  // The actual RPC.
  Status status = stub_->Login(&context, user, &replySessionID);

  WorkingDirectory = "/";

  // Act upon its status.
  if (status.ok()) {
    std::cout << "Login Test... Received session id: " << replySessionID.id()
              << std::endl;
    return replySessionID.id();
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return "0";
  }
}

std::string FtpClient::LogoutTest(std::string sessionIdStr) {
  SessionID sessionid;
  sessionid.set_id(sessionIdStr);

  FtpStatus replyStatus;

  ClientContext context;

  // The actual RPC.
  Status status = stub_->Logout(&context, sessionid, &replyStatus);

  // Act upon its status.
  if (status.ok()) {
    std::cout << "Logout Test... Received status code: " << replyStatus.code()
              << std::endl;
    return "0";
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return "1";
  }
}

std::vector<FtpClient::DirectoryVec> FtpClient::ListDirectoryTest(
    std::string sessionIdStr) {
  SessionID sessionid;
  sessionid.set_id(sessionIdStr);

  Directory replyDirectory;

  ClientContext context;

  // The actual RPC.
  Status status = stub_->ListDirectory(&context, sessionid, &replyDirectory);

  std::vector<FtpClient::DirectoryVec> listDirectoryRes;

  // Act upon its status.
  if (status.ok()) {
    RepeatedPtrField<DEntry> *firstEntry = replyDirectory.mutable_dentries();
    for (auto iter = firstEntry->begin(); iter != firstEntry->end(); ++iter) {
      FtpClient::DirectoryVec temp;
      if (iter->type() == DEntry_PathType::DEntry_PathType_DIRECTORY) {
        temp.name = iter->name();
        temp.type = "_DIR";
        temp.size = iter->size();

        listDirectoryRes.push_back(temp);
        std::cout << "ListDirectory Test... There's a directory named "
                  << iter->name() << "/ with size " << iter->size()
                  << std::endl;
      } else {
        temp.name = iter->name();
        temp.type = "_FILE";
        temp.size = iter->size();

        listDirectoryRes.push_back(temp);
        std::cout << "ListDirectory Test... There's a file named "
                  << iter->name() << " with size " << iter->size() << std::endl;
      }
    }

    return listDirectoryRes;
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return listDirectoryRes;
  }
}

std::string FtpClient::GetWorkingDirectoryTest(std::string sessionIdStr) {
  SessionID sessionid;
  sessionid.set_id(sessionIdStr);

  Path replyPath;

  ClientContext context;

  // The actual RPC.
  Status status = stub_->GetWorkingDirectory(&context, sessionid, &replyPath);

  // Act upon its status.
  if (status.ok()) {
    std::cout << "GetWorkingDirectory Test... Current path is "
              << replyPath.path() << std::endl;
    return replyPath.path();
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return "";
  }
}

std::string FtpClient::ChangeWorkingDirectoryTest(std::string sessionIdStr,
                                                  std::string directory) {
  ChangeInfo changeinfo;
  changeinfo.mutable_sessionid()->set_id(sessionIdStr);
  changeinfo.mutable_path()->set_path(directory);

  FtpStatus replyStatus;

  ClientContext context;

  // The actual RPC.
  Status status =
      stub_->ChangeWorkingDirectory(&context, changeinfo, &replyStatus);

  // Act upon its status.
  if (status.ok()) {
    std::cout << "ChangeWorkingDirectory Test... Server return with the code:"
              << replyStatus.code() << std::endl;
    return "0";
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return "1";
  }
}

std::string FtpClient::DownloadSmallFileTest(std::string sessionIdStr,
                                             std::string filepath) {
  std::cout << "Test Download" << std::endl;
  ChangeInfo changeinfo;
  changeinfo.mutable_sessionid()->set_id(sessionIdStr);
  changeinfo.mutable_path()->set_path(filepath);

  std::cout << filepath << std::endl;

  FileChunk filechunk;

  ClientContext context;

  // The actual RPC.
  Status status = stub_->DownloadSmallFile(&context, changeinfo, &filechunk);

  // Act upon its status.
  if (status.ok()) {
    std::cout << "DownloadSmallFile Test... Download the server's small file: "
              << filepath << std::endl;

    int fd = open((WorkingDirectory + '/' + filepath).c_str(),
                  O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
      std::cout << "Open File Succeed" << std::endl;
      if (write(fd, filechunk.data().c_str(), filechunk.size()) > 0) {
        std::cout << "Download Finish" << std::endl;
        close(fd);
        return "0";
      } else {
        close(fd);
        return "1";
      }
    } else {
      std::cout << "Open File Fail" << std::endl;
      return "1";
    }
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return "1";
  }
}

std::string FtpClient::UploadSmallFileTest(std::string sessionIdStr,
                                           std::string filepath) {
  std::cout << "Test Upload" << std::endl;
  FileInfo fileinfo;
  ChangeInfo temp_changeinfo;
  temp_changeinfo.mutable_sessionid()->set_id(sessionIdStr);

  temp_changeinfo.mutable_path()->set_path(filepath);

  std::cout << "set chagneinfo succeed" << std::endl;

  ChangeInfo *changeinfo = fileinfo.mutable_changeinfo();
  changeinfo->CopyFrom(temp_changeinfo);

  std::cout << "set fileinfo succeed" << std::endl;

  ClientContext context;

  FtpStatus uploadstatus;

  int fd = open((WorkingDirectory + '/' + filepath).c_str(), O_RDONLY);

  if (fd != -1) {
    struct stat st = {};
    fstat(fd, &st);
    size_t size = st.st_size;

    std::string buffer(size, ' ');
    if (read(fd, &buffer[0], size) > 0) {
      fileinfo.mutable_filechunk()->set_size(size);
      fileinfo.mutable_filechunk()->set_offset(0);
      fileinfo.mutable_filechunk()->set_data(buffer);
      std::cout << "Read File Succeed" << std::endl;
      close(fd);
    } else {
      fileinfo.mutable_filechunk()->set_size(-1);
      std::cout << "Read File Fail" << std::endl;
      close(fd);
    }
  } else {
    fileinfo.mutable_filechunk()->set_size(-1);
    std::cout << "Open File Fail" << std::endl;
  }

  // The actual RPC.
  Status status = stub_->UploadSmallFile(&context, fileinfo, &uploadstatus);
  std::cout << uploadstatus.code() << std::endl;
  // Act upon its status.
  if (status.ok() && uploadstatus.code() == 0) {
    std::cout << "UploadSmallFile Test... Upload the local's small file: "
              << WorkingDirectory + '/' + filepath << std::endl;

    return "0";
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return "1";
  }
}

/*
int main () {

}
*/