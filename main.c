#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define AUR_URL "https://aur.archlinux.org/rpc.php"
#define MAX_BUFFER 1024

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    strncat(userdata, ptr, size * nmemb);
    return size * nmemb;
}

void search_package(const char *package_name) {
    CURL *curl;
    CURLcode res;
    char buffer[MAX_BUFFER] = {0};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if(curl) {
        char url[MAX_BUFFER];
        snprintf(url, sizeof(url), "%s?type=search&arg=%s", AUR_URL, package_name);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
        
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("Response:\n%s\n", buffer);
        }
        
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <package_name>\n", argv[0]);
        return 1;
    }

    search_package(argv[1]);

    return 0;
}
