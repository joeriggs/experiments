#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <openssl/sha.h>
#include <openssl/evp.h>

static unsigned char rawKey[32] = {
   0x6D, 0x22, 0xCD, 0x59, 0x3B, 0x2A, 0x34, 0xBB, 0x74, 0x7A, 0x6B, 0x26, 0xC7, 0x27, 0x05, 0x3A,
   0x4F, 0xE8, 0x1B, 0x04, 0x98, 0x6A, 0x73, 0x45, 0xE3, 0x56, 0x6E, 0xF0, 0x82, 0xC4, 0xD4, 0x7B
};

static unsigned char ofbIVmod[16] = {
    0x1C, 0x07, 0xAF, 0x88, 0xDA, 0x79, 0x29, 0xD7, 0x64, 0x80, 0x60, 0xB6, 0x4B, 0xF0, 0xDD, 0xE9
};

static unsigned char cipherText[10] = { 0x20, 0x74, 0x5D, 0xED, 0xA0, 0xBF, 0x07, 0x9F, 0xEC, 0x2A };

static unsigned char clearText[10]  = { 0x54, 0x45, 0x58, 0x54, 0x20, 0x44, 0x41, 0x54, 0x41, 0x0A };

static void logData(unsigned char *p, size_t pLen, const char *func, const char *msg)
{
	int x;
	printf("%s(): ========== %s: %p.\n", func, msg, p);
	if((p != 0) && (pLen > 0)) {
		for(x = 0; x < pLen; x += 16)
		{
			char buf[128];
			sprintf(buf, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
			         p[x + 0], p[x + 1], p[x + 2], p[x + 3], p[x + 4], p[x + 5], p[x + 6], p[x + 7],
			         p[x + 8], p[x + 9], p[x +10], p[x +11], p[x +12], p[x +13], p[x +14], p[x +15]);
			printf("%s(): %04X: %s\n", func, x, buf);
		}
	}
	printf("%s(): ========== End of %s.\n", func, msg);
}

static unsigned char myESSIV[16];
static void createESSIV(void) {
	unsigned char md[SHA_DIGEST_LENGTH];
	SHA1(rawKey, sizeof(rawKey), md);
	//logData(md, sizeof(md), __func__, "md");

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

	int rc = EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, rawKey, NULL);
	//printf("EVP_EncryptInit_ex() returned %d.\n", rc);

	int len = 0;
	rc = EVP_EncryptUpdate(ctx, myESSIV, &len, md, sizeof(md));
	//printf("EVP_EncryptUpdate() returned %d.\n", rc);
	int outputLen = len;

	rc = EVP_EncryptFinal_ex(ctx, myESSIV + len, &len);
	//printf("EVP_EncryptFinal_ex() returned %d.\n", rc);
	//logData(myESSIV, sizeof(myESSIV), __func__, "myESSIV");
}

int main(int argc, char **arg)
{
	int retcode = 0;
	unsigned char output[512];
	memset(output, 0, sizeof(output));

	createESSIV();

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

	int rc;
	rc = EVP_DecryptInit_ex(ctx, EVP_aes_256_ofb(), NULL, rawKey, ofbIVmod);
	printf("EVP_DecryptInit_ex() returned %d.\n", rc);

	int len = 0;
	EVP_DecryptUpdate(ctx, output, &len, cipherText, sizeof(cipherText));
	printf("EVP_DecryptUpdate() returned %d.  len %d.\n", rc, len);
	int outputLen = len;

	EVP_DecryptFinal_ex(ctx, output + len, &len);
	printf("EVP_DecryptFinal_ex() returned %d.  len %d.\n", rc, len);
	outputLen += len;
	logData(output, 10, __func__, "output");

	retcode = memcmp(clearText, output, sizeof(clearText));

	return retcode;
}

