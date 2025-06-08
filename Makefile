all: build


build:
	g++ NewRouter/NewRouter.cpp NewRouter/ARPTable.cpp Device/Device.cpp -o router
	g++ NewClient/NewClient.cpp NewClient/NewClient.h Device/Device.cpp -o client
	g++ DHCP/DHCP.cpp DHCP/DHCPTable.cpp Device/Device.cpp -o dhcp
	g++ DNS/DNS.cpp Device/Device.cpp -o dns