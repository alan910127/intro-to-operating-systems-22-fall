# Directory Traversal and File Transfer

## Environment Varaibles

To run this project, you need to export the following environment variables

```bash
export PKG_CONFIG_PATH=~/.local/lib/pkgconfig
```

## Build Project

```bash
make
```

And there will be two executables `ftp_gui_im/ftpgui` and `server/server`

### Build on Your Own

#### Build Protocol Buffers

```bash
cd protos
make
```

#### Build the Server

```bash
cd server
make
```

#### Build the GUI Client

```bash
cd ftp_gui_im
qmake
make
```