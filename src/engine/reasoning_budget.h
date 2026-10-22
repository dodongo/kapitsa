#pragma once

#include <charconv>
#include <cctype>
#include <string>
#include <string_view>

namespace ni {

struct ReasoningBudget {
  int target = 0;
  int limit = 0;
  int tokens = 0;
  std::string text;

  static bool parse(std::string_view value, ReasoningBudget& out) {
    const size_t colon = value.find(':');
    if (colon == std::string_view::npos) return false;
    int target = 0, limit = 0;
    auto a = std::from_chars(value.data(), value.data() + colon, target);
    auto b = std::from_chars(value.data() + colon + 1, value.data() + value.size(), limit);
    if (a.ec != std::errc() || a.ptr != value.data() + colon || b.ec != std::errc() ||
        b.ptr != value.data() + value.size() || target < 1 || limit < target) return false;
    out = {target, limit, 0, {}};
    return true;
  }

  bool push(std::string_view delta) {
    ++tokens;
    text += delta;
    if (tokens >= limit) return true;
    if (tokens < target) return false;
    bool fenced = false;
    for (size_t at = text.find("```"); at != std::string::npos; at = text.find("```", at + 3)) fenced = !fenced;
    if (fenced) return false;
    size_t end = text.size();
    while (end > 0 && (std::isspace(static_cast<unsigned char>(text[end - 1])) ||
                       std::string_view("\"')]}").find(text[end - 1]) != std::string_view::npos)) --end;
    if (end == 0) return false;
    if (text[end - 1] == '!' || text[end - 1] == '?') return true;
    if (text[end - 1] != '.' || (end > 1 && std::isdigit(static_cast<unsigned char>(text[end - 2])))) return false;
    return true;
  }
};

}
