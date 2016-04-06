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

#include "relaycontrol.h"

#define RFIDDEVICE "/dev/hidraw0"
#define RELAYDEVICE "/dev/ttyUSB0"
#define MAXKEYBYTES 1024
#define OPENSECONDS "\x05"
#define KEYVALIDSEQ "\xAF\xFF" OPENSECONDS "\x04\xDF"
#define ACCESSREQ "TESTDOOR" /* ACCESSREQ = What this reader controls */
#define KEYSERVERURL "http://127.0.0.1/rfid/index.php" /* KEYSERVERURL = Where is rfid/index.php? */


uint64_t get_timestamp() {
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

bool validate_key(unsigned char * md5digest, char * accessreq) {
	char * md5buffer = calloc((MD5_DIGEST_LENGTH * 2) + 1, sizeof(char));
	unsigned char * expectedresponse_bin = calloc(MD5_DIGEST_LENGTH + 1, sizeof(char));
	unsigned char * expectedresponse_str = calloc((MD5_DIGEST_LENGTH * 2) + 1, sizeof(char));
	unsigned char * responsebuffer = calloc((MD5_DIGEST_LENGTH * 2) + 1, sizeof(char));
	char * postbuffer = calloc(1024, sizeof(char));

	const char * SOMETHING1 = "Kulosaari";
	const char * SOMETHING2 = "00570";

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
		sprintf(postbuffer, "k=%s&a=%s", md5buffer, accessreq);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postbuffer);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
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
	free(postbuffer);
	free(md5buffer);
	free(responsebuffer);
	free(expectedresponse_bin);
	free(expectedresponse_str);
	
	return retval;
}

int main (void) {
	int reader, serial;
	bool rd_started = false;  
	uint64_t rd_start_ts = 0;
	
	printf("Trying to open RFID reader: %s\n", RFIDDEVICE);
	reader = open(RFIDDEVICE, O_NONBLOCK | O_RDONLY);
	unsigned char * buffer = calloc(MAXKEYBYTES + 8 + 1, sizeof(unsigned char));
	unsigned int buffer_ptr = 0;

	unsigned char * md5digest = calloc(MD5_DIGEST_LENGTH + 1, sizeof(unsigned char));

	printf("Trying to open USB-serial relay: %s\n", RELAYDEVICE);
	serial = open_serial_interface(RELAYDEVICE);

	if (serial > 0) {
		int retval = setup_serial_interface(serial, B9600, 0);
		if (retval != 0) {
			printf("Serial configuration failed\n");
			exit(255);
		}
	}

	if (reader != 0 && serial > 0) {
		printf("Listening to RFID reader: %s\n", RFIDDEVICE);

		while (1) {
			int count = read(reader, buffer + buffer_ptr, 8);

			if (count > 0) {
				if (!rd_started) {
					rd_started = true;
					rd_start_ts = get_timestamp();
				}
				
				buffer_ptr += count;
			} else {
				if (rd_started) {
					usleep(1000);
				} else {
					usleep(1000 * 100);
				}
			}

			if (rd_started) {
				uint64_t ts_diff = get_timestamp() - rd_start_ts;
				if (ts_diff > 200000 || buffer_ptr >= MAXKEYBYTES) {
					rd_started = false;
					unsigned char * keybuffer;
					unsigned int keylen;
					get_key_buffer(buffer, buffer_ptr, &keybuffer, &keylen);

					key_to_md5hash(keybuffer, keylen, md5digest);

					for (int i=0; i<MD5_DIGEST_LENGTH; i++) {
						fprintf(stdout, "%02x", md5digest[i]);
					}
						
					fprintf(stdout, "\n");
					fflush(stdout);
					
					if (validate_key(md5digest, ACCESSREQ)) {
						fprintf(stdout, "%s access OK\n", ACCESSREQ);
						fflush(stdout);

						int status = write(serial, KEYVALIDSEQ, strlen(KEYVALIDSEQ));

						if (status < 0) {
							fprintf(stdout, "Opening lock failed!\n");
							fflush(stdout);
						}

					} else {
						fprintf(stdout, "Access denied for %s/", ACCESSREQ);

						for (int i=0; i<MD5_DIGEST_LENGTH; i++) {
							fprintf(stdout, "%02x", md5digest[i]);
						}
						
						fprintf(stdout, "\n");
						fflush(stdout);
					}

					buffer_ptr = 0;
					free(keybuffer);
										
				}
			}
		}

	} else {
		printf("Opening %s failed\n", RFIDDEVICE);
		exit(255);
	}
	
	return 0;
}


