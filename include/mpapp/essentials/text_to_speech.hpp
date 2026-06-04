// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::text_to_speech` — speech synthesis. Counterpart to MAUI
// Essentials `TextToSpeech`. Supports speaking text with optional
// per-call options (volume, pitch, locale) and enumeration of the
// locales available on the current platform. Abstract interface + an
// in-memory mock implementation whose recorded state is inspectable so
// tests can drive and verify TTS interactions. Real per-platform backends
// (Windows SpeechSynthesizer, Android TextToSpeech, iOS AVSpeechSynthesizer)
// implement the same interface and are injected via the DI container
// (RFC-0011). No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_TEXT_TO_SPEECH_HPP
#define MPAPP_ESSENTIALS_TEXT_TO_SPEECH_HPP

#include <optional>
#include <string>
#include <vector>

namespace mpapp {

// ---------------------------------------------------------------------------
// Value types
// ---------------------------------------------------------------------------

// Per-call speech synthesis options. Mirrors MAUI's SpeechOptions.
// volume: 0.0 (silent) – 1.0 (full volume); default 1.0.
// pitch:  0.0 (low) – 2.0 (high); default 1.0 (normal).
// locale: locale id string (e.g. "en-US"). Empty string means the
//         platform default.
struct speech_options {
    double      volume = 1.0;
    double      pitch  = 1.0;
    std::string locale{};

    bool operator==(const speech_options&) const = default;
};

// Describes a single synthesis locale. Mirrors MAUI's Locale.
// id       — opaque platform identifier (e.g. "com.apple.voice.compact.en-US.Samantha").
// name     — human-readable display name (e.g. "Samantha").
// language — BCP-47 language tag (e.g. "en").
// country  — ISO-3166 country code (e.g. "US").
struct tts_locale {
    std::string id{};
    std::string name{};
    std::string language{};
    std::string country{};

    bool operator==(const tts_locale&) const = default;
};

// ---------------------------------------------------------------------------
// Abstract interface
// ---------------------------------------------------------------------------

class text_to_speech {
public:
    virtual ~text_to_speech() = default;

    // Speak `text` using platform defaults.
    virtual void speak(const std::string& text) = 0;

    // Speak `text` with the supplied options. An empty locale string in
    // `opts` means "use platform default".
    virtual void speak(const std::string& text, const speech_options& opts) = 0;

    // Return the list of synthesis locales available on this device.
    // May be empty on platforms where locale enumeration is unsupported.
    [[nodiscard]] virtual std::vector<tts_locale> get_locales() const = 0;
};

// ---------------------------------------------------------------------------
// Mock / in-memory implementation
// ---------------------------------------------------------------------------
// Records every speak() call so tests can inspect them. Canned locales
// are settable via set_locales(). Provides spoken_texts() to inspect the
// history and last_options() for the most recent options.

class mock_text_to_speech final : public text_to_speech {
public:
    mock_text_to_speech() = default;

    // ---- Interface implementation ------------------------------------------

    void speak(const std::string& text) override {
        spoken_texts_.push_back(text);
        last_options_ = std::nullopt;
        ++speak_count_;
    }

    void speak(const std::string& text, const speech_options& opts) override {
        spoken_texts_.push_back(text);
        last_options_ = opts;
        ++speak_count_;
    }

    [[nodiscard]] std::vector<tts_locale> get_locales() const override {
        return locales_;
    }

    // ---- Test-side configuration -------------------------------------------

    // Replace the list of canned locales returned by get_locales().
    void set_locales(std::vector<tts_locale> locales) {
        locales_ = std::move(locales);
    }

    // ---- Inspection helpers ------------------------------------------------

    // Returns every text passed to either speak() overload, in call order.
    [[nodiscard]] const std::vector<std::string>& spoken_texts() const noexcept {
        return spoken_texts_;
    }

    // Returns the most recent options, or std::nullopt if the last speak()
    // was called without options (or speak() has never been called).
    [[nodiscard]] std::optional<speech_options> last_options() const noexcept {
        return last_options_;
    }

    // Returns the total number of speak() calls (both overloads combined).
    [[nodiscard]] int speak_count() const noexcept {
        return speak_count_;
    }

    // Reset all recorded state (spoken texts, last options, counter).
    // The canned locale list is NOT cleared by reset(); use set_locales({})
    // to clear it explicitly.
    void reset() noexcept {
        spoken_texts_.clear();
        last_options_ = std::nullopt;
        speak_count_  = 0;
    }

private:
    std::vector<std::string>     spoken_texts_{};
    std::optional<speech_options> last_options_{};
    int                           speak_count_ = 0;
    std::vector<tts_locale>       locales_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_TEXT_TO_SPEECH_HPP
