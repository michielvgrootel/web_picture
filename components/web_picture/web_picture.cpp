#include "web_picture.h"
#include <cstdlib>
#include <string>

namespace esphome::web_picture {

using tessera::Font;
namespace ui = tessera::ui;

// The tile: one place for a picture over the whole card, the tile's name in its bottom corner.
//
// The address is the option `url`, with `url_more` after it (a text option holds at most 64 characters, so a longer
// address is split over the two). The app fetches it from a host of the manifest's permissions.network and keeps it
// by address for hours, so to show a picture that changes at the same address (a webcam's snapshot), the tile adds
// `_t=<number>` to the query, a number that goes up every `refresh` minutes on the screen's clock: another address,
// so the app fetches it again. The place keeps the old picture until the new one is there, so it changes without a
// flash. With refresh off the address goes as it is.
class PictureTile : public tessera::Tile {
 public:
  explicit PictureTile(const tessera::Plugin *plugin) : plugin_(plugin) {}

  void create(const tessera::TileContext &c) override {
    std::string url = c.options["url"] | "";
    url += c.options["url_more"] | "";
    const std::string clean = trim(url);
    url_.assign(clean.data(), clean.size());
    every_ = 60 * static_cast<uint32_t>(std::atoi(c.options["refresh"] | "0"));
    contain_ = std::string(c.options["fit"] | "cover") == "contain";
    caption_ = c.options["caption"] | true;
    name_ = c.name;
    height_ = c.height;
    if (ui::pictures()) {
      place_ = ui::picture(c.parent);
      lv_obj_set_size(place_, c.width, c.height);
      lv_obj_set_pos(place_, 0, 0);
    }
    label_ = ui::label(c.parent, Font::BODY, theme::INK);
    lv_obj_set_width(label_, c.width);
    // No pictures on this board, or no address yet: one line in the middle that says so.
    if (!place_) say(plugin_->text("no_pictures"));
    else if (!usable()) say(plugin_->text("no_address"));
    else show_name();
  }

  void on_state(JsonObjectConst) override { ask(0); }

  // Once a second: a new address when the refresh step moved on (the same address again costs nothing).
  void on_tick(uint32_t epoch) override { ask(epoch); }

  void on_picture(lv_obj_t *place, tessera::PictureState state) override {
    if (place != place_) return;
    if (state == tessera::PictureState::SHOWN) show_name();
    else say(plugin_->text("no_picture"));
  }

  void on_theme() override { ui::set_color(label_, missing_ ? theme::MUTED : theme::INK); }

 private:
  static std::string trim(const std::string &s) {
    const auto a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
  }

  bool usable() const {
    const std::string u = roomy::plain(url_);
    return u.rfind("https://", 0) == 0 || u.rfind("http://", 0) == 0;
  }

  // The address for this moment: the option's, with the refresh step in its query when refresh is on and the clock set.
  std::string address(uint32_t epoch) const {
    std::string u = roomy::plain(url_);
    if (!every_ || !epoch) return u;
    std::string fragment;
    const auto hash = u.find('#');
    if (hash != std::string::npos) {
      fragment = u.substr(hash);
      u.erase(hash);
    }
    u += u.find('?') == std::string::npos ? "?" : "&";
    u += "_t=" + std::to_string(epoch / every_);
    return u + fragment;
  }

  void ask(uint32_t epoch) {
    if (!place_ || !usable()) return;
    // Until the clock is set, the plain address; after that the step, and only a new step makes a new address.
    const uint32_t step = every_ && epoch ? epoch / every_ : 0;
    if (asked_ && step == step_) return;
    asked_ = true;
    step_ = step;
    ui::set_picture(place_, address(epoch), contain_ ? tessera::Fit::CONTAIN : tessera::Fit::COVER);
  }

  // Words in the middle of the card, muted: why there is no picture.
  void say(const std::string &words) {
    missing_ = true;
    lv_obj_set_style_text_align(label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_y(label_, (height_ - ui::line_height(Font::BODY)) / 2);
    ui::set_text(label_, words);
    lv_obj_remove_flag(label_, LV_OBJ_FLAG_HIDDEN);
    on_theme();
  }

  // The tile's name in the bottom corner over the picture, or nothing.
  void show_name() {
    missing_ = false;
    lv_obj_set_style_text_align(label_, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_y(label_, height_ - ui::line_height(Font::BODY));
    ui::set_text(label_, caption_ ? roomy::plain(name_) : "");
    if (caption_) lv_obj_remove_flag(label_, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(label_, LV_OBJ_FLAG_HIDDEN);
    on_theme();
  }

  const tessera::Plugin *plugin_;
  tessera::Text url_, name_;   // kept as long as the card: PSRAM first (docs/FIRMWARE_API.md, "Memory")
  uint32_t every_ = 0, step_ = 0;
  bool contain_ = false, caption_ = true, missing_ = false, asked_ = false;
  int height_ = 0;
  lv_obj_t *place_{}, *label_{};
};

void WebPicture::setup() {
  add_tile("picture", [this]() { return new PictureTile(this); });
}

}  // namespace esphome::web_picture
