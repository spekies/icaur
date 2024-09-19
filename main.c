#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#define AUR_URL "https://aur.archlinux.org/rpc.php"
#define MAX_BUFFER 1024

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    strncat(userdata, ptr, size * nmemb);
    return size * nmemb;
}

void download_file(const char *url, const char *filename) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(filename, "wb");
        if (!fp) {
            fprintf(stderr, "Failed to open file %s for writing\n", filename);
            return;
        }
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        fclose(fp);
        curl_easy_cleanup(curl);
    }
}

void search_package(const char *package_name) {
    CURL *curl;
    CURLcode res;
    char buffer[MAX_BUFFER] = {0};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if(curl) {
        char url[MAX_BUFFER];
        snprintf(url, sizeof(url), "%s?v=5&type=search&arg=%s", AUR_URL, package_name);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
        
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("Response:\n%s\n", buffer);
            
            cJSON *json = cJSON_Parse(buffer);
            if (json) {
                cJSON *results = cJSON_GetObjectItem(json, "results");
                if (cJSON_GetArraySize(results) > 0) {
                    cJSON *pkg = cJSON_GetArrayItem(results, 0);
                    const char *pkgbase = cJSON_GetObjectItem(pkg, "Name")->valuestring;

                    char pkgbuild_url[MAX_BUFFER];
                    snprintf(pkgbuild_url, sizeof(pkgbuild_url), "https://aur.archlinux.org/cgit/aur.git/plain/PKGBUILD?h=%s", pkgbase);

                    printf("Downloading PKGBUILD for %s...\n", pkgbase);
                    download_file(pkgbuild_url, "PKGBUILD");
                } else {
                    printf("No packages found for '%s'.\n", package_name);
                }
                cJSON_Delete(json);
            } else {
                printf("Error parsing JSON response.\n");
            }
        }
        
        // Cleanup
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("- - ICAUR - -\n");
        fprintf(stderr, "Usage: %s <package_name>\n", argv[0]);
        return 1;
    }

    search_package(argv[1]);

    return 0;
}
