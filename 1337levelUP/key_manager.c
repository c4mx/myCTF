#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/un.h>

#include <mbedtls/rsa.h>
#include <mbedtls/sha256.h>
#include <mbedtls/aes.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/pk.h>

#define PAGE_SIZE 0x1000

int decrypt_file(void *);
int encrypt_file(void *);
int enable_secure_mode(void *);

//-------------- SHM helpers ---------------------

key_t get_key()
{
    static uint32_t n = 1;
    return (key_t)n++;
}

int create_shm(uint32_t nb_pages)
{
    key_t key = get_key();
    int ret;
    if ((ret = shmget(key, nb_pages * PAGE_SIZE, IPC_CREAT | 0660)) == -1)
    {
        perror("create_shm");
        abort();
    }
    return ret;
}

void *attach_shm(int handle)
{
    void *addr = shmat(handle, NULL, 0);
    if ((long)(addr) == -1)
    {
        perror("shmat");
        abort();
    }
    return addr;
}

void detach_shm(void *addr)
{
    int ret = shmdt(addr);
    if (ret == -1)
    {
        perror("shmdt");
        abort();
    }
}

void destroy_shm(int handle)
{
    int ret = shmctl(handle, IPC_RMID, NULL);
    if (ret == -1)
    {
        perror("destry shm");
        abort();
    }
}

//-----------------------------------------------

#define NB_SHM_MAX 100
#define NB_MAX_CLIENTS 10

enum request_id
{
    REQ_ALLOC_SHM,
    REQ_DECRYPT_FILE,
    REQ_ENCRYPT_FILE,
    REQ_ENABLE_SECURE_MODE,
    REQ_FREE_SHM
};

enum resp_code
{
    RESP_OK,
    ERROR_BAD_REQ,
    ERROR_TOO_MUCH_SHM,
    ERROR_SHM_NOT_CREATED,
    ERROR_COMM,
    ERROR_BAD_ARG,
    ERROR_DECRYPT
};

enum handle_res
{
    HANDLE_OK,
    HANDLE_CONN_CLOSE,
    HANDLE_UNK_ERROR
};

struct req_alloc_shm_data
{
    uint32_t nb_pages;
};

struct req_shm_data
{
    uint32_t shm_handle;
};

struct shms
{
    int handles[NB_SHM_MAX];
    int nb_shms;
};

struct client_ctxt
{
    int socket;
    struct shms handles;
};

struct service
{
    struct client_ctxt *clients[NB_MAX_CLIENTS];
    int nb_clients;
    int list_socket;
};

typedef enum SEC_MODE
{
    NORMAL,
    SECURE
} SEC_MODE;

struct service *create_service(int socket)
{
    struct service *service = (struct service *)malloc(sizeof(struct service));
    if (!service)
    {
        perror("malloc");
        abort();
    }
    memset(service, 0, sizeof(struct service));
    service->list_socket = socket;
    return service;
}

void service_add_client(struct service *service, struct client_ctxt *client)
{
    for (int i = 0; i < NB_MAX_CLIENTS; i++)
    {
        if (!service->clients[i])
        {
            service->clients[i] = client;
            service->nb_clients++;
            break;
        }
    }
}

void service_del_client(struct service *service, struct client_ctxt *client)
{
    for (int i = 0; i < NB_MAX_CLIENTS; i++)
    {
        if (service->clients[i] == client)
        {
            service->clients[i] = NULL;
            service->nb_clients--;
            break;
        }
    }
}

void client_init_handles(struct client_ctxt *client)
{
    for (int i = 0; i < NB_SHM_MAX; i++)
    {
        client->handles.handles[i] = -1;
    }
    client->handles.nb_shms = 0;
}

struct client_ctxt *create_client(int socket)
{
    struct client_ctxt *client = (struct client_ctxt *)malloc(sizeof(struct client_ctxt));
    if (!client)
    {
        perror("malloc");
        abort();
    }
    client->socket = socket;
    client_init_handles(client);
    return client;
}

void destroy_client(struct client_ctxt *client)
{
    close(client->socket);
    for (int i = 0; i < NB_SHM_MAX; i++)
    {
        if (client->handles.handles[i] != -1)
        {
            destroy_shm(client->handles.handles[i]);
        }
    }
    mode = NORMAL;
    free(client);
}

void client_add_new_handle(struct client_ctxt *client, int handle)
{
    for (int i = 0; i < NB_SHM_MAX; i++)
    {
        if (client->handles.handles[i] == -1)
        {
            client->handles.handles[i] = handle;
            client->handles.nb_shms++;
            break;
        }
    }
}

void client_del_handle(struct client_ctxt *client, int handle)
{
    for (int i = 0; i < NB_SHM_MAX; i++)
    {
        if (client->handles.handles[i] == handle)
        {
            client->handles.handles[i] = -1;
            client->handles.nb_shms--;
            break;
        }
    }
}

int client_handle_exists(struct client_ctxt *client, int handle)
{
    for (int i = 0; i < NB_SHM_MAX; i++)
    {
        if (client->handles.handles[i] == handle)
        {
            return true;
        }
    }
    return false;
}

int handle_alloc_shm(struct client_ctxt *client)
{
    struct req_alloc_shm_data data;
    int shm_handle;

    int n = read(client->socket, &data, sizeof(data));
    if (n != sizeof(data))
    {
        return ERROR_COMM;
    }

    if (!(data.nb_pages && data.nb_pages < 4))
    {
        return ERROR_BAD_ARG;
    }

    if (client->handles.nb_shms == NB_SHM_MAX)
    {
        return ERROR_TOO_MUCH_SHM;
    }

    shm_handle = create_shm(data.nb_pages);
    client_add_new_handle(client, shm_handle);

    n = write(client->socket, &shm_handle, sizeof(shm_handle));
    if (n != sizeof(shm_handle))
    {
        return ERROR_COMM;
    }
    return RESP_OK;
}

int handle_free_shm(struct client_ctxt *client)
{
    struct req_shm_data data;

    int n = read(client->socket, &data, sizeof(data));
    if (n != sizeof(data))
    {
        return ERROR_COMM;
    }

    if (client_handle_exists(client, data.shm_handle))
    {
        return ERROR_BAD_ARG;
    }

    destroy_shm(data.shm_handle);
    client_del_handle(client, data.shm_handle);
    return RESP_OK;
}

int handle_decrypt_file(struct client_ctxt *client)
{
    struct req_shm_data data;
    void *addr;
    int ret;

    int n = read(client->socket, &data, sizeof(data));
    if (n != sizeof(data))
    {
        return ERROR_COMM;
    }

    if (!client_handle_exists(client, data.shm_handle))
    {
        return ERROR_BAD_ARG;
    }

    addr = attach_shm(data.shm_handle);
    ret = decrypt_file(addr);

    if (ret)
    {
        return ERROR_DECRYPT;
    }

    detach_shm(addr);

    return RESP_OK;
}

int handle_encrypt_file(struct client_ctxt *client)
{
    struct req_shm_data data;
    void *addr;
    int ret;

    int n = read(client->socket, &data, sizeof(data));
    if (n != sizeof(data))
    {
        return ERROR_COMM;
    }

    if (!client_handle_exists(client, data.shm_handle))
    {
        return ERROR_BAD_ARG;
    }

    addr = attach_shm(data.shm_handle);
    ret = encrypt_file(addr);

    if (ret)
    {
        return ERROR_DECRYPT;
    }

    detach_shm(addr);

    return RESP_OK;
}

int handle_enable_secure_mode(struct client_ctxt *client)
{
    struct req_shm_data data;
    void *addr;
    int ret;

    int n = read(client->socket, &data, sizeof(data));
    if (n != sizeof(data))
    {
        return ERROR_COMM;
    }

    if (!client_handle_exists(client, data.shm_handle))
    {
        return ERROR_BAD_ARG;
    }

    addr = attach_shm(data.shm_handle);
    ret = enable_secure_mode(addr);

    if (ret)
    {
        return ERROR_DECRYPT;
    }

    detach_shm(addr);

    return RESP_OK;
}

enum handle_res handle_request(struct client_ctxt *client)
{
    enum request_id req;
    enum resp_code resp;
    int n = read(client->socket, &req, sizeof(req));
    if (n != sizeof(req))
    {
        return HANDLE_CONN_CLOSE;
    }
    switch (req)
    {
    case REQ_ALLOC_SHM:
        resp = handle_alloc_shm(client);
        break;
    case REQ_DECRYPT_FILE:
        resp = handle_decrypt_file(client);
        break;
    case REQ_ENCRYPT_FILE:
        resp = handle_encrypt_file(client);
        break;
    case REQ_ENABLE_SECURE_MODE:
        resp = handle_enable_secure_mode(client);
        break;
    case REQ_FREE_SHM:
        resp = handle_free_shm(client);
        break;
    default:
        resp = ERROR_BAD_REQ;
    }
    if (resp == ERROR_COMM)
    {
        return HANDLE_CONN_CLOSE;
    }

    n = write(client->socket, &resp, sizeof(resp));
    if (n != sizeof(resp))
    {
        return HANDLE_CONN_CLOSE;
    }

    return HANDLE_OK;
}

int create_listening_socket(char *name)
{
    struct sockaddr_un server_sockaddr;
    int server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    int ret;
    if (server_sock == -1)
    {
        perror("socket");
        abort();
    }

    server_sockaddr.sun_family = AF_UNIX;
    strncpy(server_sockaddr.sun_path, name, sizeof(server_sockaddr.sun_path));

    unlink(name);
    ret = bind(server_sock, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr));
    if (ret == -1)
    {
        perror("bind");
        close(server_sock);
        abort();
    }

    ret = listen(server_sock, 10);
    if (ret == -1)
    {
        perror("listen");
        close(server_sock);
        abort();
    }

    return server_sock;
}

int main()
{
    struct service *service;

    int listening_socket = create_listening_socket("/tmp/secure_storage.unix");
    service = create_service(listening_socket);

    while (true)
    {
        fd_set readfds;
        enum handle_res res;
        int nfds = 0, ret;
        FD_ZERO(&readfds);
        FD_SET(service->list_socket, &readfds);
        nfds = service->list_socket;
        for (int i = 0; i < NB_MAX_CLIENTS; i++)
        {
            if (service->clients[i])
            {
                FD_SET(service->clients[i]->socket, &readfds);
                if (service->clients[i]->socket > nfds)
                {
                    nfds = service->clients[i]->socket;
                }
            }
        }
        nfds += 1;
        ret = select(nfds, &readfds, NULL, NULL, NULL);
        if (ret == -1)
        {
            perror("select");
            abort();
        }
        else if (ret)
        {
            if (FD_ISSET(service->list_socket, &readfds))
            {
                int new_socket = accept(service->list_socket, NULL, 0);
                if (service->nb_clients >= NB_MAX_CLIENTS)
                {
                    close(new_socket);
                }
                struct client_ctxt *client = create_client(new_socket);
                service_add_client(service, client);
            }
            for (int i = 0; i < NB_MAX_CLIENTS; i++)
            {
                if (service->clients[i])
                {
                    if (FD_ISSET(service->clients[i]->socket, &readfds))
                    {
                        struct client_ctxt *client = service->clients[i];
                        res = handle_request(client);
                        if (res == HANDLE_CONN_CLOSE)
                        {
                            service_del_client(service, client);
                            destroy_client(client);
                        }
                    }
                }
            }
        }
    }
    return 0;
}

//----------------------- Decrypt file -----------------------------

#define SIG_SIZE 0x100
#define CHUNK_SIZE 0x20

#define COMMON_KEY <redacted>
#define SECURE_KEY "xxxxxxxxxxxxxxxx"
#define RSA_KEY_FILE "rsa_key.bin"
#define IV_PERS "THIS IS AN IV"
#define SHM_BASE 0x0
#define SHM_SIZE 0xffffffffffffffff

typedef struct __attribute__((__packed__)) SECFILE_
{
    uint8_t signature[SIG_SIZE];
    uint8_t is_secure;
    uint8_t reserved[7];
    // each chunk is 0x20 bytes
    uint64_t chunk_count;
    uint8_t iv[16];
    uint8_t data[0];
} SECFILE;

#define SEC_FILE_HDR_SIZE (sizeof(SECFILE))
typedef struct RAWDATA_
{
    size_t size;
    uint8_t data;
} RAWDATA;

SEC_MODE mode;
typedef struct TOKEN_
{
    uint8_t UUID[64];
    uint8_t signature[SIG_SIZE];
} TOKEN;

int in_shm(void *ptr, size_t size)
{
    if (((uint64_t)ptr < SHM_BASE) || ((uint64_t)ptr > (SHM_BASE + SHM_SIZE)))
    {
        return 0;
    }
    uint64_t end = ((uint64_t)((uint8_t)ptr) + size);
    if ((end < SHM_BASE) || (end > (SHM_BASE + SHM_SIZE)))
    {
        return 0;
    }
    return 1;
}

int mbedtls_load_rsa_key(mbedtls_pk_context *pk_ctx, mbedtls_ctr_drbg_context *ctr_drbg)
{
    int ret;
    mbedtls_pk_init(pk_ctx);
    if ((ret = mbedtls_pk_parse_keyfile(pk_ctx, RSA_KEY_FILE, "",
                                        mbedtls_ctr_drbg_random, ctr_drbg)) != 0)
    {
        printf("Could not read key from '%s'\n", RSA_KEY_FILE);
        printf("mbedtls_pk_parse_public_keyfile returned %d\n", ret);
        goto fail;
    }
    if (!mbedtls_pk_can_do(pk_ctx, MBEDTLS_PK_RSA))
    {
        printf("Key is not an RSA key\n");
        goto fail;
    }
    if ((ret = mbedtls_rsa_set_padding(mbedtls_pk_rsa(*pk_ctx),
                                       MBEDTLS_RSA_PKCS_V21,
                                       MBEDTLS_MD_SHA256)) != 0)
    {
        printf("Padding not supported\n");
        goto fail;
    }
fail:
    return ret;
}

int decrypt_file(void *ptr)
{
    mbedtls_pk_context pk_ctx;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_sha256_context hash_ctx;
    mbedtls_aes_context aes_ctx;
    unsigned char sha_file[32];
    SECFILE *file = (SECFILE *)ptr;
    uint8_t *clear = NULL;
    int ret;
    // select key
    mbedtls_aes_init(&aes_ctx);
    if (file->is_secure)
    {
        mbedtls_aes_setkey_dec(&aes_ctx, (const unsigned char *)SECURE_KEY, 128);
    }
    else
    {
        mbedtls_aes_setkey_dec(&aes_ctx, (const unsigned char *)COMMON_KEY, 128);
    }
    // compute hash
    mbedtls_sha256_init(&hash_ctx);
    mbedtls_sha256_starts(&hash_ctx, 0);
    mbedtls_sha256_update(&hash_ctx, &file->is_secure, SEC_FILE_HDR_SIZE - SIG_SIZE + file->chunk_count * CHUNK_SIZE);
    mbedtls_sha256_finish(&hash_ctx, sha_file);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_load_rsa_key(&pk_ctx, &ctr_drbg);
    // check signature
    if ((ret = mbedtls_pk_verify(&pk_ctx, MBEDTLS_MD_SHA256, sha_file, 0,
                                 file->signature, SIG_SIZE)) != 0)
    {
        printf("Invalid signature for secure file.\n");
        goto fail;
    }
    if (file->is_secure && mode != SECURE)
    {
        printf("You are not authorized to decypt this file\n");
        return -1;
    }
    clear = malloc(file->chunk_count * CHUNK_SIZE);
    if (clear == NULL)
    {
        printf("Out of memory.");
        ret = -1;
        goto fail;
    }
    ret = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, CHUNK_SIZE * file->chunk_count, file->iv, &file->data, clear);
    if (ret != 0)
    {
        printf("Decryption error.\n");
        goto fail;
    }
    for (int i = 0; i < file->chunk_count; i++)
    {
        memcpy(&file->data + i * CHUNK_SIZE, clear + i * CHUNK_SIZE, CHUNK_SIZE);
    }
    free(clear);
    printf("Decryption done.\n");
    return 0;

fail:
    if (clear != NULL)
    {
        free(clear);
    }
    return ret;
}

int enable_secure_mode(void *ptr)
{
    TOKEN *token;
    mbedtls_pk_context pk_ctx;
    mbedtls_ctr_drbg_context ctr_drbg;
    unsigned char sha_uuid[32];
    token = (TOKEN *)ptr;
    if (!in_shm(token, sizeof(TOKEN)))
    {
        printf("Invalid token location");
        return -1;
    }
    mbedtls_ctr_drbg_init(&ctr_drbg);
    int ret;
    mbedtls_pk_init(&pk_ctx);
    if ((ret = mbedtls_pk_parse_keyfile(&pk_ctx, RSA_KEY_FILE, "",
                                        mbedtls_ctr_drbg_random, &ctr_drbg)) != 0)
    {
        printf("Could not read key from '%s'\n", RSA_KEY_FILE);
        printf("mbedtls_pk_parse_public_keyfile returned %d\n", ret);
        goto fail;
    }
    if (!mbedtls_pk_can_do(&pk_ctx, MBEDTLS_PK_RSA))
    {
        printf("Key is not an RSA key\n");
        goto fail;
    }
    if ((ret = mbedtls_rsa_set_padding(mbedtls_pk_rsa(pk_ctx),
                                       MBEDTLS_RSA_PKCS_V21,
                                       MBEDTLS_MD_SHA256)) != 0)
    {
        printf("Padding not supported\n");
        goto fail;
    }
    if ((ret = mbedtls_md(
             mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
             token->UUID, 64, sha_uuid)) != 0)
    {
        printf("mbedtls_md error\n");
        goto fail;
    }
    if ((ret = mbedtls_pk_verify(&pk_ctx, MBEDTLS_MD_SHA256, sha_uuid, 0,
                                 token->signature, SIG_SIZE)) != 0)
    {
        printf("Invalid signature for token file.\n");
        goto fail;
    }
    mode = SECURE;
    printf("Secure mode enabled\n");
fail:
    return ret;
}

int encrypt_file(void *ptr)
{
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    mbedtls_aes_context aes_ctx;
    mbedtls_pk_context pk_ctx;

    unsigned char sha_file[32];
    unsigned char iv_cpy[16];
    uint64_t chunk_count;
    void *clear = NULL;
    SECFILE *secfile = NULL;
    int ret;
    size_t sig_len;
    RAWDATA *raw = (RAWDATA *)ptr;
    if (!in_shm(ptr, sizeof(RAWDATA) - sizeof(uint8_t)))
    {
        printf("Input data does not fit in SHM.\n");
        return -1;
    }
    size_t size = raw->size;
    if (!in_shm(&raw->data, size))
    {
        printf("Input data does not fit in SHM.\n");
        return -1;
    }
    mbedtls_aes_init(&aes_ctx);
    if (mode == SECURE)
    {
        mbedtls_aes_setkey_enc(&aes_ctx, (const unsigned char *)SECURE_KEY, 128);
    }
    else
    {
        mbedtls_aes_setkey_enc(&aes_ctx, (const unsigned char *)COMMON_KEY, 128);
    }
    chunk_count = size / CHUNK_SIZE;
    if (size % CHUNK_SIZE)
    {
        chunk_count += 1;
    }
    // check that there is enogh space in SHM to accomodate the secure file
    if (!in_shm(ptr, SEC_FILE_HDR_SIZE + CHUNK_SIZE * chunk_count))
    {
        printf("SecureFile does not fit in SHM.\n");
        return -1;
    }
    secfile = malloc(SEC_FILE_HDR_SIZE + CHUNK_SIZE * chunk_count);
    if (secfile == NULL)
    {
        printf("Cannot allocate memory.\n");
        goto fail;
    }
    secfile->chunk_count = chunk_count;
    memset(&secfile->data, 0, CHUNK_SIZE * secfile->chunk_count);

    secfile->is_secure = mode == SECURE ? 1 : 0;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)IV_PERS, strlen(IV_PERS))) != 0)
    {
        printf("mbedtls_ctr_drbg_init returned -0x%04x\n", -ret);
        goto fail;
    }
    if ((ret = mbedtls_ctr_drbg_random(&ctr_drbg, secfile->iv, 16)) != 0)
    {
        printf("mbedtls_ctr_drbg_random returned -0x%04x\n", -ret);
        goto fail;
    }
    clear = malloc(CHUNK_SIZE * secfile->chunk_count);
    if (clear == NULL)
    {
        printf("Cannot allocate memory.\n");
        goto fail;
    }
    memset(clear, 0, CHUNK_SIZE * secfile->chunk_count);
    memcpy(clear, &raw->data, size);
    memcpy(iv_cpy, secfile->iv, 16);
    ret = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, CHUNK_SIZE * secfile->chunk_count, iv_cpy, clear, &secfile->data);
    free(clear);
    if ((ret = mbedtls_md(
             mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
             &secfile->is_secure, SEC_FILE_HDR_SIZE - SIG_SIZE + CHUNK_SIZE * secfile->chunk_count, sha_file)) != 0)
    {
        printf("mbedtls_md error\n");
        goto fail;
    }
    mbedtls_load_rsa_key(&pk_ctx, &ctr_drbg);
    if ((ret = mbedtls_pk_sign(&pk_ctx, MBEDTLS_MD_SHA256, sha_file, 32,
                               secfile->signature, SIG_SIZE, &sig_len,
                               mbedtls_ctr_drbg_random, &ctr_drbg)) != 0)
    {
        printf("mbedtls_pk_sign returned %d\n", ret);
        goto fail;
    }
    memcpy(ptr, secfile, SEC_FILE_HDR_SIZE + secfile->chunk_count * CHUNK_SIZE);
    printf("Encryption done.\n");
    free(secfile);
    return 0;

fail:
    if (clear != NULL)
    {
        free(clear);
    }
    if (secfile != NULL)
    {
        free(secfile);
    }
    return -1;
}
