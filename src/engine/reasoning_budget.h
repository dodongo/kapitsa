#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

namespace ni {

struct ReasoningBudget {
  int target = 0;
  int limit = 0;
  int tokens = 0;
  std::string text;

  static ReasoningBudget for_request(std::string_view effort, int max_tokens, int requested, int close_tokens) {
    if (effort == "none" || requested == 0) return {};
    ReasoningBudget out{1536, 2048, 0, {}};
    if (effort == "minimal") out = {96, 128, 0, {}};
    else if (effort == "low") out = {256, 320, 0, {}};
    else if (effort == "medium") out = {768, 1024, 0, {}};
    if (requested > 0) out.limit = std::min(out.limit, requested);
    const int total = std::max(0, max_tokens);
    const int room = total - std::min(1024, std::max(1, total / 2)) - close_tokens;
    out.limit = std::max(0, std::min(out.limit, room));
    out.target = std::min(out.target, out.limit);
    return out;
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
