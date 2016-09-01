#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

#include <openssl/md5.h>
#include <curl/curl.h>

#include "cJSON.h"

#include "serialsetup.h"

#define RFIDDEVICE "/dev/ttyUSB1"
#define RELAYDEVICE "/dev/ttyUSB0"
#define KEYVALIDSEQ "\xAF\xFF\x05\x04\xDF"
#define RFIDOKSEQ "ok\n"
#define ACCESSREQ "SIDEDOOR" /* ACCESSREQ = What this reader controls */
#define KEYSERVERURL "https://example.com/rfid/index.php" /* KEYSERVERURL = Where is rfid/index.php? */
#define HASHING1 "SOMETHING1"
#define HASHING2 "SOMETHING2"
#define HTTPAUTH_PASSWORD "password12345"

uint64_t get_timestamp(void) {
	struct timeval t;
	gettimeofday(&t, NULL);
	return 1000000 * t.tv_sec + t.tv_usec;
}

void get_key_buffer(unsigned char * in_buf, unsigned int in_buf_len, unsigned char ** out_buf, unsigned int * out_buf_len) {
	unsigned char * tmpbuff = calloc((in_buf_len >> 4) + 1, sizeof(unsigned char));
	unsigned int tmpbuff_ptr = 0;
	
	for (int i=0; i<in_buf_len >> 4; i++) {
		tmpbuff[tmpbuff_ptr++] = in_buf[i * 16 + 2];
	}
	
	 * out_buf_len = tmpbuff_ptr;
	 * out_buf = tmpbuff;
}

void key_to_md5hash(unsigned char * in_buf, unsigned int in_buf_len, unsigned char * out_buf) {
	MD5_CTX md5ctx;
	MD5_Init(&md5ctx);
	MD5_Update(&md5ctx, in_buf, in_buf_len);
	MD5_Final(out_buf, &md5ctx);
}

size_t header_write_func(void *ptr, size_t size, size_t nmemb, void *userdata) {
	char keyname[] = "Keyhash: ";
	char hash[40];
	
	if (nmemb > strlen(keyname) && nmemb < (40 + strlen(keyname))) {
		if (strncmp(keyname, (char *)ptr, strlen(keyname)) == 0) {
			sscanf((char *)ptr, "Keyhash: %s", hash);
			fprintf(stdout, "got: %s\n", hash);
			fflush(stdout);

			if (strlen(hash) == MD5_DIGEST_LENGTH * 2) {
				memcpy(userdata, hash, MD5_DIGEST_LENGTH * 2);
			}
		}
	}

	return size * nmemb;
}

bool validate_key(unsigned char * md5digest, const char * accessreq, char * pincode) {
	char * md5buffer = calloc((MD5_DIGEST_LENGTH * 2) + 1, sizeof(char));
	unsigned char * expectedresponse_bin = calloc(MD5_DIGEST_LENGTH + 1, sizeof(char));
	unsigned char * expectedresponse_str = calloc((MD5_DIGEST_LENGTH * 2) + 1, sizeof(char));
	unsigned char * responsebuffer = calloc((MD5_DIGEST_LENGTH * 2) + 1, sizeof(char));
	char * postbuffer = calloc(1024, sizeof(char));
	char * authbuffer = calloc(1024, sizeof(char));

	const char * SOMETHING1 = HASHING1;
	const char * SOMETHING2 = HASHING2;

	CURL *curl;
	CURLcode res;
	long http_status = 0;
	bool retval = false;
	
	for (int i=0; i<MD5_DIGEST_LENGTH; i++) {
		sprintf(md5buffer + (i * 2), "%02x", md5digest[i]);
	}

	fprintf(stdout, "Validating %s/%s\n", accessreq, md5buffer);
	fflush(stdout);
	
	unsigned int expectedlen = strlen(SOMETHING1) + strlen(md5buffer) + strlen(accessreq) + strlen(SOMETHING2);
	
	unsigned char * tmpbuf = calloc(expectedlen + 1, sizeof(unsigned char));
	snprintf((char *)tmpbuf, expectedlen + 1, "%s%s%s%s", SOMETHING1, md5buffer, accessreq, SOMETHING2);

	key_to_md5hash(tmpbuf, expectedlen, expectedresponse_bin);
	free(tmpbuf);

	for (int i=0; i<MD5_DIGEST_LENGTH; i++) {
		sprintf((char *) expectedresponse_str + (i * 2), "%02x", expectedresponse_bin[i]);
	}
	
	fprintf(stdout, "exp: %s\n", expectedresponse_str);
	fflush(stdout);

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, KEYSERVERURL);
		sprintf(postbuffer, "k=%s&a=%s&p=%s", md5buffer, accessreq, pincode);
		fprintf(stdout, "Query: %s\n", postbuffer);
		fflush(stdout);
		
		sprintf(authbuffer, "%s:%s", accessreq, HTTPAUTH_PASSWORD);

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postbuffer);
		curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);
		curl_easy_setopt(curl, CURLOPT_USERPWD, authbuffer);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, header_write_func);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)responsebuffer);
		res = curl_easy_perform(curl);
		
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);
		
		if (http_status == 200 && res != CURLE_ABORTED_BY_CALLBACK) {
			if (strlen((char *)responsebuffer) == MD5_DIGEST_LENGTH * 2) {
				if (strncmp((char *)responsebuffer, (char *)expectedresponse_str, MD5_DIGEST_LENGTH * 2) == 0) {
					retval = true;
				}
			}
		}
		
		curl_easy_cleanup(curl);
	}
	
	curl_global_cleanup();
	free(authbuffer);
	free(postbuffer);
	free(md5buffer);
	free(responsebuffer);
	free(expectedresponse_bin);
	free(expectedresponse_str);
	
	return retval;
}

int main (void) {
	int RFIDReaderFD, RELAYdeviceFD;
	bool rd_started = false;
	
	printf("Trying to open RFID reader: %s\n", RFIDDEVICE);
	RFIDReaderFD = open_serial_interface(RFIDDEVICE);
	
	if (RFIDReaderFD > 0) {
		int retval = setup_serial_interface(RFIDReaderFD, B9600, 0);
		if (retval != 0) {
			printf("Reader serial configuration failed\n");
			exit(255);
		}
	}
	
	unsigned char * md5digest = calloc(MD5_DIGEST_LENGTH + 1, sizeof(unsigned char));

	printf("Trying to open USB-serial relay: %s\n", RELAYDEVICE);
	RELAYdeviceFD = open_serial_interface(RELAYDEVICE);

	if (RELAYdeviceFD > 0) {
		int retval = setup_serial_interface(RELAYdeviceFD, B9600, 0);
		if (retval != 0) {
			printf("Serial configuration failed\n");
			exit(255);
		}
	}

	if (RFIDReaderFD != 0 && RELAYdeviceFD > 0) {
		printf("Listening to RFID reader: %s\n", RFIDDEVICE);

		char readbuffer[1024];
		char jsonbuffer[1024];
		int jsonbufferptr = 0;
		
		while (1) {
			ssize_t length = read(RFIDReaderFD, &readbuffer, sizeof(readbuffer));
			
			if (length == -1) {
				printf("Error reading RFID reader: %s\n", RFIDDEVICE);
			} else if (length == 0) {
				if (rd_started) {
					usleep(10000);
				} else {
					usleep(1000 * 100);
				}
			} else {
				if (!rd_started) {
					rd_started = true;
				}
				
				for (int i=0; i<length; i++) {
					if (readbuffer[i] != '\n') {
						jsonbuffer[jsonbufferptr++] = readbuffer[i];
					} else {
						cJSON *json = NULL;
						
						if (strlen(jsonbuffer) > 0) {
							jsonbuffer[jsonbufferptr] = '\0';
							printf("READ: %s\n", jsonbuffer);
						
							json = cJSON_Parse(jsonbuffer);
							
							if (!json) {
								printf("Error before: [%s]\n",cJSON_GetErrorPtr());
							}
						}
						
						if (json) {
							if (cJSON_HasObjectItem(json, "status")) {
								char * status = cJSON_GetObjectItem(json, "status")->valuestring;
								printf("status: %s\n", status);
							
							} else if (cJSON_HasObjectItem(json, "ping")) {
								double ping = cJSON_GetObjectItem(json, "ping")->valuedouble;
								printf("ping: %f\n", ping);

							} else if (cJSON_HasObjectItem(json, "keycode") && cJSON_HasObjectItem(json, "pincode")) {
								char * keycode = cJSON_GetObjectItem(json, "keycode")->valuestring;
								char * pincode = cJSON_GetObjectItem(json, "pincode")->valuestring;
							
								fprintf(stdout, "Keycode: %s\n", keycode);
								fprintf(stdout, "Pincode: %s\n", pincode);

								key_to_md5hash((unsigned char *)keycode, strlen(keycode), md5digest);

								fprintf(stdout, "Keyhash: ");

								for (int i=0; i<MD5_DIGEST_LENGTH; i++) {
									fprintf(stdout, "%02x", md5digest[i]);
								}

								fprintf(stdout, "\n");
								fflush(stdout);
							
								if (validate_key(md5digest, ACCESSREQ, pincode)) {
									fprintf(stdout, "%s access OK\n\n", ACCESSREQ);
									fflush(stdout);

									int status = write(RELAYdeviceFD, KEYVALIDSEQ, strlen(KEYVALIDSEQ));

									if (status < 0) {
										fprintf(stdout, "Opening lock failed!\n");
										fflush(stdout);
									}
									
									status = write(RFIDReaderFD, RFIDOKSEQ, strlen(RFIDOKSEQ));
									
									if (status < 0) {
										fprintf(stdout, "Sending ok to rfidreader failed!\n");
										fflush(stdout);
									}


								} else {
									fprintf(stdout, "Access denied for %s/", ACCESSREQ);

									for (int i=0; i<MD5_DIGEST_LENGTH; i++) {
										fprintf(stdout, "%02x", md5digest[i]);
									}
						
									fprintf(stdout, "\n\n");
									fflush(stdout);
								}
							
								cJSON_Delete(json);
							}
							
						}
						
						rd_started = false;
						
						memset(jsonbuffer, 0, sizeof(jsonbuffer) * sizeof(char));
						jsonbufferptr = 0;
					}
				}
				
			}
		}

	} else {
		printf("Opening %s failed\n", RFIDDEVICE);
		exit(255);
	}
	
	return 0;
}


