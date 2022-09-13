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
__attribute__((section(".ram_code"))) uint8_t get_hex_rep(char *str_input, uint8_t in_size, uint8_t *hex_out)
{
	if(str_input == NULL || hex_out == NULL)
	{
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
__attribute__((section(".ram_code"))) int amp_aes_init(private_firev_data_t *priv_data)
{
	if(priv_data==NULL)
	{
		return -1;
	}
	/* Initing nonce generator */

	int result = 1;

	uint8_t entropy[256] = {0x7f, 0x40, 0x80, 0x46, 0x93, 0x55, 0x2e, 0x31, 0x75, 0x23, 0xfd, 0xa6, 0x93, 0x5a, 0x5b, 0xc8, 0x14, 0x35, 0x3b, 0x1f
							, 0xbb, 0x7d, 0x33, 0x49, 0x64, 0xac, 0x4d, 0x1d, 0x12, 0xdd, 0xcc, 0xce};

	result = tc_ctr_prng_init(&ctx, &entropy[0], sizeof(entropy), NULL, 0);

	if (result != 1) {
		return -2;
	}

	// TODO: Proper way of defining the key
	const uint8_t nist_key[KEY_SIZE_BYTES] = {
		0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
		0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
	};

	/* Setting the key */
	memcpy(&priv_data->key[0], &nist_key[0], KEY_SIZE_BYTES);


	return 0;
}

/**
 * @brief Function to get a new nonce
 *
 * @param data_ctrl 	The pointer for the struct holding the information to perform the encryption
 * @return int 			Returns 0 if success, < 0 if an error ocurred
 */
__attribute__((section(".ram_code"))) int amp_aes_update_nonce(private_firev_data_t *priv_data)
{
	if(priv_data==NULL)
	{
		return -1 ;

	}

	int result = 1;

	result = tc_ctr_prng_generate(&ctx, NULL, 0, &priv_data->nonce[0], NONCE_SIZE);

	if (result != 1) {
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
__attribute__((section(".ram_code"))) int amp_aes_encrypts(shared_data_t *data_ctrl, private_firev_data_t *priv_d)
{
	if(data_ctrl==NULL || priv_d == NULL)
	{
		return -1 ;

	}
	else if (data_ctrl->flag==1)
	{
		/* Setting encryption configs */
		uint8_t text_len = CLASS_SIZE;
		uint8_t cipher_size = text_len;

		mbedtls_gcm_context ctx;

		mbedtls_gcm_init(&ctx);

		int result = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, &priv_d->key[0], KEY_SIZE_BITS);
		if (result == MBEDTLS_ERR_GCM_BAD_INPUT){
			return -2;
		}

		
		/* Encryption phase */
		result = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, cipher_size, &priv_d->nonce[0], NONCE_SIZE, NULL, 0, &data_ctrl->predicted_class, &data_ctrl->ciphertext[0], MAC_LEN,  &data_ctrl->tag[0]);
		if (result == MBEDTLS_ERR_GCM_BAD_INPUT) {
			return -3;
		}

		/* Releasing the flag */
		data_ctrl->flag = 0;
		mbedtls_gcm_free(&ctx);
		return 0;

	}else{
		return -4;
	}
}