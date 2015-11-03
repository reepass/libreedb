#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "crypto/engine.h"
#include "crypto/hashes.h"
#include "utils/files.h"
#include "ree_vault.h"


/* This is called in case of a failure */
ree_err_t undo_directories()
{
	return SUCCESS;
}

ree_err_t create_folder_structure(char *name, char *path)
{
	int folder;
	char *new_name = rdb_concat_fname_simple(name, ".reevault");
	char *master = rdb_concat_path_simple(path, new_name);
	free(new_name);

	/* Top directory */
	folder = mkdir(master, 0755);
	if(folder != 0)
	{
		if(RDB_DEBUG) printf("The path provided was not empty! Code %d\n", folder);
		return VAULT_CREATE_FAILED;
	}

	/* Create the keystore directory */
	char *keystore = rdb_concat_path_simple(master, "keystore");
	folder = mkdir(keystore, 0700);
	if(folder != 0) goto param_failure;
	free(keystore);

	/* Create the keystore directory */
	char *datastore = rdb_concat_path_simple(master, "datastore");
	folder = mkdir(datastore, 0755);
	if(folder != 0) goto param_failure;
	free(datastore);

	/* Create the keystore directory */
	char *parity = rdb_concat_path_simple(master, "parity");
	folder = mkdir(parity, 0755);
	if(folder != 0) goto param_failure;
	free(parity);

	/* Clean up after ourselves */
	free(master);
	return SUCCESS;

/** Error handling label */
param_failure:
	printf("Folder create error code %d, invalid parameters. Name: %s, Path: %s", 
		folder, name, path);
	int undo = undo_directories();
	return VAULT_CREATE_FAILED;
}

ree_err_t rdb_create_vault(vault *(*vault), char *uuid, char *name, char *path, char *passphrase)
{
	/* Create the vault folders and report errors upstairs */
	int fdscss = create_folder_structure(name, path);
	if(fdscss != 0) return fdscss;

	/* Malloc a vault struct */
	(*vault) = malloc(sizeof(struct vault));
	if((*vault) == NULL) return MALLOC_FAILED;

	/* Let's start with some metadata */
	(*vault)->id = uuid;
	(*vault)->size = 0;
	(*vault)->name = name;
	(*vault)->path = path;
	// (*vault)->created = Time.now
	// (*vault)->modified = Time.now

	/* Make some room for the crypto storage */
	(*vault)->keystore = hashmap_new();
	(*vault)->tokens = hashmap_new();
	
	/* Then alloc the actual datastorage */
	(*vault)->tags = hashmap_new();
	(*vault)->files = hashmap_new();

	/* Next up let's expand our base key */
	unsigned char *salt;
	int success = rcry_random_secure(&salt, RCRY_SALT_LENGTH, 999);
	
	unsigned char *key_pad;
	success = rcry_hash_tiger2(passphrase, &key_pad, salt);

	/* Now create a cryptographically secure key and encrypt it with the user pw */
	unsigned char *key;
	unsigned char *encrypted_key;
	success = rcry_keygen(&key, AUTO_USE);
	if(success != 0)
	{
		fputs("Creating a key has failed horribly!\n", stderr);
		goto failure_handle;
	}

	int index = 0;
	int text_length = strlen(key);
	printf("Our lovely key: ");
	for (index = 0; index < text_length; index++)
		printf("%02X", (unsigned char) key[index]);
	printf("\n");

	return success;

	// success = rcry_encrypt(STRING, key_pad, &encrypted_key, key);
	// if(success != 0)
	// {
	// 	fputs("Doing encryption has failed horribly!\n", stderr);
	// 	goto failure_handle;
	// }

	// return success;


failure_handle:
	undo_directories();
	free(key_pad);
	free(salt);
	free(key);
	//TODO: Add rcry_crypto_term(...) or something

	return VAULT_CREATE_FAILED;
}

ree_err_t rdb_dump_vault(vault *next, vault *vault)
{
	return SUCCESS;
}


