#include "main.h"

#include <archive.h>
#include <archive_entry.h>
#include <curl/curl.h>
#include <dlfcn.h>
#include <isocline.h>
#include <stdio.h>
#include <unistd.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

[[nodiscard]] bool copy_data(struct archive* ar, struct archive* aw) {
  int r;
  const void* buff;
  size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
  int64_t offset;
#else
  off_t offset;
#endif

  for (;;) {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF) return (true);
    if (r != ARCHIVE_OK) return (false);
    r = archive_write_data_block(aw, buff, size, offset);
    if (r != ARCHIVE_OK) {
      fprintf(stderr, "\e[0;33mwarning: %s\e[0;37m\n",
              archive_error_string(aw));
      return (false);
    }
  }
}
[[nodiscard]] bool extract(std::string prefix, std::string filename) {
  struct archive* a;
  struct archive* ext;
  struct archive_entry* entry;
  int r;

  a = archive_read_new();
  ext = archive_write_disk_new();
  archive_write_disk_set_options(ext, 0);
  archive_read_support_filter_gzip(a);
  archive_read_support_format_tar(a);
  if ((r = archive_read_open_filename(a, filename.c_str(), 10240))) {
    fprintf(stderr, "\e[0;31merror: %s\e[0;37m\n", archive_error_string(a));
    return false;
  }

  for (;;) {
    r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF) break;
    if (r != ARCHIVE_OK) {
      fprintf(stderr, "\e[0;31merror: %s\e[0;37m\n", archive_error_string(a));
      return false;
    }

    auto pathname = std::string(archive_entry_pathname(entry));
    if (pathname.starts_with(prefix)) {
      pathname = pathname.substr(prefix.size(), pathname.size());
    }
    archive_entry_set_pathname(entry, pathname.c_str());
    printf("\e[0;37mextracted \e[0;32m%s\e[0;37m\n", pathname.c_str());

    r = archive_write_header(ext, entry);

    if (r == ARCHIVE_OK) {
      if (!copy_data(a, ext)) {
        fprintf(stderr, "\e[0;31merror: couldn't decompress\e[0;37m\n");
        return false;
      }
      r = archive_write_finish_entry(ext);
      if (r != ARCHIVE_OK) {
        fprintf(stderr, "\e[0;31merror: %s\e[0;37m\n",
                archive_error_string(ext));
        return false;
      }
    }
  }
  archive_read_close(a);
  archive_read_free(a);

  archive_write_close(ext);
  archive_write_free(ext);
  return true;
}

int progress_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                      curl_off_t ultotal, curl_off_t ulnow) {
  draw_flower(dltotal != 0 ? (double)dlnow / (double)dltotal : 0.0, true);
  return 0;
}

size_t write_string(char* ptr, size_t /*unused*/, size_t nmemb,
                    void* userdata) {
  auto* user_str = (std::string*)userdata;
  user_str->insert(user_str->end(), ptr, ptr + nmemb);
  return nmemb;
}

std::expected<bool, std::string> grab_file(std::string file,
                                           std::string output_filename) {
  CURLcode result;
  CURL* curl;

  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
  curl_easy_setopt(curl, CURLOPT_URL, file.c_str());
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/8.20.0");
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(curl, CURLOPT_SSLVERSION, (long)CURL_SSLVERSION_TLSv1_2);
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

  auto out_fd = fopen(output_filename.c_str(), "w");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, out_fd);

  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, CURLFOLLOW_ALL);

  result = curl_easy_perform(curl);
  fclose(out_fd);

  curl_easy_cleanup(curl);
  curl = NULL;

  if (result != CURLE_OK) {
    return std::unexpected("failed to fetch tarball");
  }

  draw_flower(1.0, false);

  return true;
}

std::expected<nlohmann::json, std::string> grab_latest_releases() {
  CURLcode result;
  CURL* curl;
  struct curl_slist* slist1;

  slist1 = NULL;
  slist1 = curl_slist_append(slist1, "Accept: application/vnd.github+json");
  slist1 = curl_slist_append(slist1, "X-GitHub-Api-Version: 2026-03-10");

  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
  curl_easy_setopt(
      curl, CURLOPT_URL,
      "https://api.github.com/repos/FoxMoss/bash-llvm/releases/latest");
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/8.20.0");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(curl, CURLOPT_SSLVERSION, (long)CURL_SSLVERSION_TLSv1_2);
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

  std::string out_json;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_string);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_json);

  result = curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  curl = NULL;
  curl_slist_free_all(slist1);
  slist1 = NULL;

  if (result != CURLE_OK) {
    return std::unexpected("failed to fetch releases");
  }

  auto releases = nlohmann::json::parse(out_json);
  if (!releases["assets"].is_array()) {
    return std::unexpected("latest release does not have assets");
  }

  if (!releases["tag_name"].is_string()) {
    return std::unexpected("latest release does not have a tag name");
  }

  return releases;
}

int main(int argc, char* argv[]) noexcept {
  auto triplet = std::format("{}-{}-{}", get_build(), get_os(), get_abi());

  printf("\e[0;31mfetching latest release...\e[0;37m\n\e[1A\e[0J");
  auto releases = grab_latest_releases();
  if (!releases.has_value()) {
    fprintf(stderr, "\e[0;32merror: %s\n\e[0;37m", releases.error().c_str());
    return 1;
  }
  auto release = std::format("bash-llvm-{}-{}-{}-{}.tar.gz",
                             releases.value()["tag_name"].get<std::string>(),
                             get_build(), get_os(), get_abi());
  auto prefix = std::format("bash-llvm-{}-{}-{}-{}/",
                            releases.value()["tag_name"].get<std::string>(),
                            get_build(), get_os(), get_abi());

  char* c_home = getenv("HOME");
  std::string install_location = "/";
  if (c_home != NULL) {
    install_location = std::format("{}/.bash-llvm/", c_home);
  }

  ic_set_prompt_marker(" ", NULL);

  while (true) {
    printf("\e[0;37mbash-llvm settings:\n");
    printf("release: \e[0;32m%s\e[0;37m\n", release.c_str());
    printf("install location: \e[0;32m%s\e[0;37m\n", install_location.c_str());
    printf("\n1) proceed with install");
    printf("\n2) change install directory");
    printf("\n3) quit install");
    printf("\n> ");
    fflush(stdout);
    fflush(stdin);

    char c = '\0';
    c = getchar();

    switch (c) {
      case '1':
        goto breakwhile;
      case '2':
        printf("\n");
        install_location = ic_readline(
            std::format(
                "\e[0;37mnew install location (currently \e[0;32m{}\e[0;37m)",
                install_location)
                .c_str());
        printf("\n");
        break;
      case '3':
      default:
        fprintf(stderr, "\e[0;31mquitting...\e[0;37m\n");
        return 1;
    }
    continue;
  breakwhile:
    break;
  }

  std::optional<std::string> download_url;
  for (auto release_asset : releases.value()["assets"]) {
    if (release_asset["name"].is_string() && release_asset["name"] == release) {
      download_url = release_asset["browser_download_url"];
      break;
    }
  }

  if (!download_url.has_value()) {
    fprintf(stderr, "\e[0;31merror: release %s doesn't have a build\n\e[0;37m",
            release.c_str());
    return 1;
  }

  std::error_code error;
  if (std::filesystem::exists(install_location, error)) {
    if (!std::filesystem::remove_all(install_location, error)) {
      fprintf(stderr,
              "\e[0;31merror: could not remove previous %s\n%s\n\e[0;37m",
              install_location.c_str(), error.message().c_str());
      return 1;
    }
  }

  if (!std::filesystem::create_directory(install_location, error)) {
    fprintf(stderr, "\e[0;31merror: could not create %s\n%s\n\e[0;37m",
            install_location.c_str(), error.message().c_str());
    return 1;
  }

  auto grabbed_file = grab_file(download_url.value(),
                                std::format("{}{}", install_location, release));
  if (!grabbed_file.has_value()) {
    fprintf(stderr, "\e[0;31merror: %s\n\e[0;37m",
            grabbed_file.error().c_str());
    return 1;
  }

  auto old_path = std::filesystem::current_path();
  std::filesystem::current_path(install_location);
  if (!extract(prefix, std::format("{}{}", install_location, release))) {
    fprintf(stderr, "\e[0;31merror: could not extract\n\e[0;37m");
  }
  std::filesystem::current_path(old_path);

  if (!std::filesystem::remove(std::format("{}{}", install_location, release),
                               error)) {
    fprintf(stderr,
            "\e[0;31merror: could not remove release file\n%s\n\e[0;37m",
            error.message().c_str());
    return 1;
  }

  printf("\n\e[0;32minstall successful!\n\e[0;37m");
  printf("\e[0;37mto complete add \e[0;32m%s\e[0;37m to your path\n\e[0;37m",
         std::format("{}bin", install_location).c_str());

  return 0;
}
