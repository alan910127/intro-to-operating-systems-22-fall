FTP_GUI_DIR := ftp_gui_im
FTP_GUI := $(FTP_GUI_DIR)/ftp_gui

SERVER_DIR := server
SERVER  := $(SERVER_DIR)/server

PROTOS_DIR := protos

.PHONY: all clean

all: $(FTP_GUI) $(SERVER)

$(FTP_GUI):
	make -C $(PROTOS_DIR)
	cd $(FTP_GUI_DIR) && qmake
	make -C $(FTP_GUI_DIR)

$(SERVER):
	make -C $(SERVER_DIR)

clean:
	-make -C $(FTP_GUI_DIR) distclean
	-make -C $(PROTOS_DIR) clean
	-make -C $(SERVER_DIR) clean
