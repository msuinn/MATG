#include <libusb-1.0/libusb.h>
#include <iostream>

using namespace std;


//Function Prototype
void printdev(libusb_device *dev);




int main(int, char**){
	//List of devices
	libusb_device **devs;
	//A libusb session, each one independant of another
	libusb_context *ctx = NULL;
	//Device handle to interact with device
	libusb_device_handle *dev_handle;
	//Return value for error checking
	int r;
	//Number of found devices
	ssize_t cnt;
	//Vendor ID of the device, should change per board
	const int vid = 10755;
	//Product ID of the device, should change per board
	const int pid = 67;
	//Max size of each data transfer
	const int dataSize = 6;
	//Data pointer for transfer
	unsigned char *data = new unsigned char[dataSize];
	//Which interface endpoint to use, found in EP Address field
	const int epAddr = 4;
	//How many bytes are written/read in bulk transfer
	int bytesWritten = 0, bytesRead = 0;



	//Initialize and quit if fails
	r = libusb_init(&ctx);
	if(r < 0){
		cout << "Init Error " << r << endl;
		delete[] data;
		return(1);
	}

	//Set the libusb library to verbose mode
	libusb_set_debug(ctx, 3);

	//Get a list of length cnt of all the USB devices, or error
	cnt = libusb_get_device_list(ctx, &devs);
	if(cnt < 0){
		cout << "Get Device Error" << endl;
	}

	//Print out the specs of each device in list
	cout << cnt << " Devices in list." << endl;
	ssize_t i;
	for(i = 0; i < cnt; i++){
		printdev(devs[i]);
	}


	//Attempt to open communication
	dev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);
	if(dev_handle == NULL){
		cout << "Cannot open communication with board" << endl;
	}



	//Check for a kernal driver attached to device
	//If a driver is attached, interface cannot be claimed and no IO
	if(libusb_kernel_driver_active(dev_handle, 1) == 1){
		cout << "Kernal driver present" << endl;
		if(libusb_detach_kernel_driver(dev_handle, 1) == 0){
					cout << "Kernal driver detached" << endl;
		}	
	}

	//Claim the device interface for IO, boards tend to have just 1
	r = libusb_claim_interface(dev_handle, 1);
	if(r < 0){
		cout << "Cannot claim interface" << endl;
	}


	//Set the data to be written
	data[0] = 'H';
	data[1] = 'e';
	data[2] = 'l';
	data[3] = 'l';
	data[4] = 'o';
	data[5] = '?';

	//Show data
	cout << "Data->" << data << "<-" << endl;

	//Write the data over the interface
	//epAddr found in EP Address field in device enumeration
	//ENPOINT_OUT means write to this endpoint
	//The last 0 in arg means infinite (no) timeout, else is timeout in ms
	r = libusb_bulk_transfer(dev_handle, (epAddr | LIBUSB_ENDPOINT_OUT), data, dataSize, 			&bytesWritten, 0);
	if(r == 0 && bytesWritten == dataSize)	{
		cout << "Write successful" << endl;
	} else{
		cout << "Write failed" << endl;
	}



	//Similar, except reading in on the other enpoint address of the device
	r = libusb_bulk_transfer(dev_handle, (131 | LIBUSB_ENDPOINT_IN), data, dataSize, 			&bytesRead, 0);
	if(r == 0)	{
		cout << "Read successful" << endl;
	} else{
		cout << "Read failed" << endl;
		cout << libusb_error_name(r) << endl;
	}
	cout << "Data Read: " << data << endl;


	//Clean up
	//For whatever reasone, this is the only clean up that can fail
	r = libusb_release_interface(dev_handle, 1);
	if(r!=0) {
		cout << "Cannot release interface" << endl;
		delete[] data;
		return(1);
	}
	
	//All these guys are void
	libusb_free_device_list(devs, 1);
	libusb_close(dev_handle);
	libusb_exit(ctx);
	delete[] data;
	

	return(0);

}

void printdev(libusb_device *dev){
	libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);

	if(r < 0){
		cout << "Failed to get device descriptor" << endl;
		return;
	}

	cout << "Number of possible configurations: " << (int)desc.bNumConfigurations << endl;
	cout << "Device Class: " << (int)desc.bDeviceClass << endl;
	cout << "Vendor ID: " << desc.idVendor << endl;
	cout << "Product ID: " << desc.idProduct << endl;
	
	libusb_config_descriptor *config;
	libusb_get_config_descriptor(dev, 0, &config);

	cout << "Interfaces: " << (int)config->bNumInterfaces << endl;
	
	const libusb_interface *inter;
	const libusb_interface_descriptor *interdesc;
	const libusb_endpoint_descriptor *epdesc;

	for(int i = 0; i < (int)config->bNumInterfaces; i++){
		inter = &config->interface[i];
		cout << "Number of alternate settings: " << inter->num_altsetting << endl;
		for(int j = 0; j < inter->num_altsetting; j++){
			interdesc = &inter->altsetting[j];
			cout << "Interface Number: " << (int)interdesc->bInterfaceNumber << endl;
			cout << "Number of endpoints: " << (int)interdesc->bNumEndpoints << endl;
			for(int k = 0; k < (int)interdesc->bNumEndpoints; k++){
				epdesc = &interdesc->endpoint[k];
				cout << "Descriptor Type: " << (int)epdesc->bDescriptorType << endl;
				cout << "EP Address: " << (int)epdesc->bEndpointAddress << endl;
			}
		}
	}

	cout << endl << endl << endl;
	libusb_free_config_descriptor(config);
	

	
}






























