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
#ifndef CLIENT_H
#define CLIENT_H
#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include "tiny_ftp.grpc.pb.h"

using ::google::protobuf::RepeatedPtrField;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using tiny_ftp::ChangeInfo;
using tiny_ftp::DEntry;
using tiny_ftp::DEntry_PathType;
using tiny_ftp::Directory;
using tiny_ftp::FtpServer;
using tiny_ftp::FtpStatus;
using tiny_ftp::Path;
using tiny_ftp::SessionID;
using tiny_ftp::User;

class FtpClient {
 public:
  FtpClient(std::shared_ptr<Channel> channel);

  struct DirectoryVec {
    std::string type;
    std::string name;
    int size;
  };

  // Assembles the client's payload, sends it and presents the response back
  // from the server.

  std::string LoginTest(std::string userName, std::string userPasswd);

  std::string LogoutTest(std::string sessionIdStr);

  std::vector<FtpClient::DirectoryVec> ListDirectoryTest(
      std::string sessionIdStr);

  std::string GetWorkingDirectoryTest(std::string sessionIdStr);

  std::string ChangeWorkingDirectoryTest(std::string sessionIdStr,
                                         std::string directory);

 private:
  std::unique_ptr<FtpServer::Stub> stub_;
};
#endif