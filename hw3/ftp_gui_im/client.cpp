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

#include <iostream>
#include <vector>

FtpClient::FtpClient(std::shared_ptr<Channel> channel)
    : stub_(FtpServer::NewStub(channel)) {}

std::string FtpClient::LoginTest(std::string userName, std::string userPasswd) {
  User user;
  user.set_name(userName);
  user.set_pwd(userPasswd);

  SessionID replySessionID;

  ClientContext context;

  // The actual RPC.
  Status status = stub_->Login(&context, user, &replySessionID);

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
/*
int main () {

}
*/