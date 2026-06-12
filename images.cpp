
#include "main.h"
#include <string>
#include <variant>
#include <vector>

std::vector<std::string> process_lines(const std::string& text) {
  std::vector<std::string> lines;

  size_t color_iter = 0;
  auto next_iter = text.find('\n', 0);
  while (color_iter != std::variant_npos) {
    auto next_iter = text.find('\n', color_iter + 1);

    if (next_iter == std::variant_npos) {
      lines.emplace_back(text.begin() + color_iter + 1, text.end());
    } else {
      lines.emplace_back(text.begin() + color_iter + 1,
                         text.begin() + next_iter);
    }
    color_iter = next_iter;
  }

  return lines;
}

void draw_flower(float prog, bool clear) {
  std::vector<std::string> non_color_lines = process_lines(non_color_image);
  std::vector<std::string> color_lines = process_lines(color_image);

  size_t max_len = std::min(non_color_lines.size(), color_lines.size());
  size_t swap_point = (prog)*max_len;
  for (size_t i = 0; i < max_len; i++) {
    if (i < max_len - swap_point) {
      printf("%s", non_color_lines[i].c_str());
    } else {
      printf("%s", color_lines[i].c_str());
    }
    printf("\n");
  }
  if (clear) {
    for (size_t lines = 0; lines < max_len; lines++) {
      printf("\e[1A");
    }
  }
}
