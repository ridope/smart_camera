/** @file aes.c
 *  @brief Source file performing the AES-128 Encryption
 *
 *  @author Lucas Esteves <lucas.esteves-rocha@insa-rennes.fr>
 */

#include "aes.h"

TCCtrPrng_t ctx;


/**
 * @brief Get the hex representation of the input string
 *
 * @param str_input 	String input
 * @param in_size 	Input size
 * @param hex_out 	Output hex representation
 * @return uint8_t 	The bytes written in the output
 */
uint8_t get_hex_rep(char *str_input, uint8_t in_size, uint8_t *hex_out)
{
	if(str_input == NULL || hex_out == NULL)
	{
		printf("\e[91;1mNull pointers\e[0m\n");
		return 0;
	}

	int out_size = 0;

	char temp_str[3];
	temp_str[2] = '\0';

	for(int i = 0; i < in_size; i+=2)
	{
		if(str_input[i]==0 || str_input[i+1]==0)
		{
			break;
		}

		temp_str[0] = str_input[i];
		temp_str[1] = str_input[i+1];

		hex_out[out_size] = strtol(&temp_str[0],NULL,16);

		out_size++;
	}

	return out_size;
}


/**
 * @brief Function to initialize the PRNG and to generate the nonce
 *
 * @param data_ctrl 	The pointer for the struct holding the information to perform the encryption
 * @return int 			Returns 0 if success, < 0 if an error ocurred
 */
int amp_aes_init(DATA *data_ctrl)
{
	if(data_ctrl==NULL)
	{
		return -1 ;

	}
	/* Initing nonce generator */

	int result = 1;

	uint8_t entropy[256] = {0x7f, 0x40, 0x80, 0x46, 0x93, 0x55, 0x2e, 0x31, 0x75, 0x23, 0xfd, 0xa6, 0x93, 0x5a, 0x5b, 0xc8, 0x14, 0x35, 0x3b, 0x1f
							, 0xbb, 0x7d, 0x33, 0x49, 0x64, 0xac, 0x4d, 0x1d, 0x12, 0xdd, 0xcc, 0xce};

	result = tc_ctr_prng_init(&ctx, &entropy[0], sizeof(entropy), NULL, 0);

	if (result != 1) {
		printf("\e[91;1mError in the PRNG init\e[0m\n");
		return -2;
	}

	// TODO: Proper way of defining the key
	const uint8_t nist_key[KEY_SIZE_BYTES] = {
		0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
		0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
	};

	/* Setting the key */
	memcpy(&data_ctrl->key[0], &nist_key[0], KEY_SIZE_BYTES);


	return 0;
}

/**
 * @brief Function to get a new nonce
 *
 * @param data_ctrl 	The pointer for the struct holding the information to perform the encryption
 * @return int 			Returns 0 if success, < 0 if an error ocurred
 */
int amp_aes_update_nonce(DATA *data_ctrl)
{
	if(data_ctrl==NULL)
	{
		return -1 ;

	}

	int result = 1;

	result = tc_ctr_prng_generate(&ctx, NULL, 0, &data_ctrl->nonce[0], NONCE_SIZE);

	if (result != 1) {
		printf("\e[91;1mError in the Nonce generation\e[0m\n");
		return -2;
	}

	return 0;
}

/**
 * @brief Encryption top function
 *
 * @param data_ctrl 		The pointer for the struct holding the information to perform the encryption
 * @return int 				Returns 0 if success, < 0 if an error ocurred
 */
int amp_aes_encrypts(DATA *data_ctrl)
{
	if(data_ctrl==NULL)
	{
		return -1 ;

	}
	else if (data_ctrl->flag==1)
	{
		uint8_t tag[MAC_LEN];

		/* Setting encryption configs */
		uint8_t text_len = CLASS_SIZE;
		uint8_t cipher_size = text_len;
		uint8_t *ciphertext = malloc(cipher_size);

		mbedtls_gcm_context ctx;

		mbedtls_gcm_init(&ctx);

		int result = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, &data_ctrl->key[0], KEY_SIZE_BITS);
		if (result == MBEDTLS_ERR_GCM_BAD_INPUT){
			return -2;
			printf("\e[91;1mError setting the encryption key\e[0m\n");
		}

		/* Encryption phase */
		result = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, cipher_size, &data_ctrl->nonce[0], NONCE_SIZE, NULL, 0, &data_ctrl->predicted_class, ciphertext, MAC_LEN, &tag[0]);
		if (result == MBEDTLS_ERR_GCM_BAD_INPUT) {
			return -3;
			printf("\e[91;1mError in the text encryption\e[0m\n");
		}

		/* Releasing the flag */
		data_ctrl->flag = 0;

		/* Displaying */
		printf("\e[94;1mNonce: \e[0m");
		for(int i=0; i < NONCE_SIZE; i++)
		{
			printf("%02x", data_ctrl->nonce[i]);
		}

		printf("\n");

		printf("\e[94;1mTag: \e[0m");
		for(int i=0; i < MAC_LEN; i++)
		{
			printf("%02x", tag[i]);
		}

		printf("\n");


		printf("\e[94;1mChiper text: \e[0m");
		for(int i=0; i < cipher_size; i++)
		{
			printf("%02x", ciphertext[i]);
		}

		printf("\n");


		return 0;

	}else{
		return -4;
	}


}

/**
 * @brief Decryption top function
 *
 *  @param data_ctrl 		The pointer for the struct holding the information to perform the encryption
 *
 */
void amp_aes_decrypts(DATA *data_ctrl, uint8_t *nonce, uint8_t *text)
{
	/* Setting decryption configs */
	uint8_t input_len = strlen((char *) text);
	uint8_t cipher_len = input_len/2;
	uint8_t text_len = cipher_len - MAC_LEN;

	uint8_t *text_out = malloc(cipher_len)+1;
	uint8_t *ciphertext = malloc(cipher_len);

	if (get_hex_rep((char *) text, input_len, ciphertext) == 0){
		printf("\e[91;1mError converting the ciphertext\e[0m\n");
		return;
	}

	mbedtls_gcm_context ctx;

	mbedtls_gcm_init(&ctx);

	int result = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, &data_ctrl->key[0], KEY_SIZE_BITS);
	if (result == MBEDTLS_ERR_GCM_BAD_INPUT){
		printf("\e[91;1mError setting the decryption key\e[0m\n");
	}

	/* Decryption phase */
	result = mbedtls_gcm_auth_decrypt(&ctx, text_len, &nonce[0], NONCE_SIZE, NULL, 0, &ciphertext[0], MAC_LEN, &ciphertext[MAC_LEN], text_out);
	if (result == MBEDTLS_ERR_GCM_BAD_INPUT) {
		printf("\e[91;1mError in the text decryption\e[0m\n");
	}

	text_out[cipher_len] = '\0';

	printf("\e[94;1mText: \e[0m");
	printf("%d\n", *text_out);
}