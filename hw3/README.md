# Client Server Communication

Simple FTP functionalities implemented using gRPC

## Environment Varaibles

To run this project, you will need to export the following environment variables

```bash
export PKG_CONFIG_PATH=~/.local/lib/pkgconfig
```

## Build Protocol Buffers

```bash
cd protos
make
```

## Build the Server

```bash
cd server
make
```

## Build the GUI Client

```bash
cd ftp_gui_im
qmake
make
```