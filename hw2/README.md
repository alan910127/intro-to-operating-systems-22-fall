# FTP Client GUI Design

## Setup

1. Install dependencies
    ```sh
    sudo apt-get update && sudo apt-get upgrade
    sudo apt -y install build-essential openssl libssl-dev libssl1.0 libgl1-mesa-dev libqt5x11extras5
    ```
2. Install Qt
    - Download [Qt5 installer](https://www.qt.io/download-qt-installer)
    - Install by running online installer

## Compile

```sh
qmake
make
```

> If `qmake` cannot be found, add alias to qmake by:
>
> `alias qmake=/path/to/qt/{version}/gcc_64/bin/qmake`

## Run

```sh
./ftp_gui
```