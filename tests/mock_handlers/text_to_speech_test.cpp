// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::text_to_speech (RFC-0013 Essentials).

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/text_to_speech.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// Default-constructed state
// ---------------------------------------------------------------------------

TEST_CASE("mock_text_to_speech starts with no spoken texts and zero count",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;

    // Act / Assert (nothing called yet)
    CHECK(tts.speak_count() == 0);
    CHECK(tts.spoken_texts().empty());
    CHECK_FALSE(tts.last_options().has_value());
    CHECK(tts.get_locales().empty());
}

// ---------------------------------------------------------------------------
// speak(text) - default overload
// ---------------------------------------------------------------------------

TEST_CASE("speak(text) records the text in spoken_texts",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;

    // Act
    tts.speak("Hello, world");

    // Assert
    REQUIRE(tts.spoken_texts().size() == 1);
    CHECK(tts.spoken_texts()[0] == "Hello, world");
    CHECK(tts.speak_count() == 1);
}

TEST_CASE("speak(text) leaves last_options as nullopt",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;

    // Act
    tts.speak("no options");

    // Assert
    CHECK_FALSE(tts.last_options().has_value());
}

TEST_CASE("speak(text) called multiple times accumulates entries",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;

    // Act
    tts.speak("one");
    tts.speak("two");
    tts.speak("three");

    // Assert
    REQUIRE(tts.spoken_texts().size() == 3);
    CHECK(tts.spoken_texts()[0] == "one");
    CHECK(tts.spoken_texts()[1] == "two");
    CHECK(tts.spoken_texts()[2] == "three");
    CHECK(tts.speak_count() == 3);
}

TEST_CASE("speak(text) accepts empty string",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;

    // Act
    tts.speak("");

    // Assert
    REQUIRE(tts.spoken_texts().size() == 1);
    CHECK(tts.spoken_texts()[0] == "");
    CHECK(tts.speak_count() == 1);
}

// ---------------------------------------------------------------------------
// speak(text, options) - options overload
// ---------------------------------------------------------------------------

TEST_CASE("speak(text, opts) records text and stores last_options",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    speech_options opts;
    opts.volume = 0.8;
    opts.pitch  = 1.2;
    opts.locale = "en-US";

    // Act
    tts.speak("Hello with options", opts);

    // Assert
    REQUIRE(tts.spoken_texts().size() == 1);
    CHECK(tts.spoken_texts()[0] == "Hello with options");
    CHECK(tts.speak_count() == 1);
    REQUIRE(tts.last_options().has_value());
    CHECK(tts.last_options()->volume == 0.8);
    CHECK(tts.last_options()->pitch  == 1.2);
    CHECK(tts.last_options()->locale == "en-US");
}

TEST_CASE("speak(text, opts) with default-constructed options",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    speech_options opts{};   // volume=1.0, pitch=1.0, locale=""

    // Act
    tts.speak("default opts", opts);

    // Assert
    REQUIRE(tts.last_options().has_value());
    CHECK(tts.last_options()->volume == 1.0);
    CHECK(tts.last_options()->pitch  == 1.0);
    CHECK(tts.last_options()->locale == "");
}

TEST_CASE("speak(text, opts) with zero volume and pitch",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    speech_options opts;
    opts.volume = 0.0;
    opts.pitch  = 0.0;

    // Act
    tts.speak("silent", opts);

    // Assert
    REQUIRE(tts.last_options().has_value());
    CHECK(tts.last_options()->volume == 0.0);
    CHECK(tts.last_options()->pitch  == 0.0);
}

TEST_CASE("speak(text, opts) with locale only",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    speech_options opts;
    opts.locale = "fr-FR";

    // Act
    tts.speak("Bonjour", opts);

    // Assert
    REQUIRE(tts.last_options().has_value());
    CHECK(tts.last_options()->locale == "fr-FR");
}

TEST_CASE("last_options reflects the MOST RECENT speak(text, opts) call",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    speech_options first_opts;
    first_opts.volume = 0.5;
    speech_options second_opts;
    second_opts.volume = 0.9;
    second_opts.locale = "de-DE";

    // Act
    tts.speak("first", first_opts);
    tts.speak("second", second_opts);

    // Assert - last call wins
    REQUIRE(tts.last_options().has_value());
    CHECK(tts.last_options()->volume == 0.9);
    CHECK(tts.last_options()->locale == "de-DE");
    CHECK(tts.speak_count() == 2);
}

TEST_CASE("speak(text) after speak(text, opts) clears last_options",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    speech_options opts;
    opts.volume = 0.7;
    tts.speak("with opts", opts);
    REQUIRE(tts.last_options().has_value());

    // Act
    tts.speak("without opts");

    // Assert
    CHECK_FALSE(tts.last_options().has_value());
    CHECK(tts.speak_count() == 2);
}

TEST_CASE("both speak overloads accumulate into spoken_texts",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    speech_options opts;
    opts.locale = "ja-JP";

    // Act
    tts.speak("plain");
    tts.speak("with opts", opts);
    tts.speak("plain again");

    // Assert
    REQUIRE(tts.spoken_texts().size() == 3);
    CHECK(tts.spoken_texts()[0] == "plain");
    CHECK(tts.spoken_texts()[1] == "with opts");
    CHECK(tts.spoken_texts()[2] == "plain again");
    CHECK(tts.speak_count() == 3);
    CHECK_FALSE(tts.last_options().has_value());  // last call had no opts
}

// ---------------------------------------------------------------------------
// get_locales + set_locales
// ---------------------------------------------------------------------------

TEST_CASE("get_locales returns empty vector by default",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;

    // Act / Assert
    CHECK(tts.get_locales().empty());
}

TEST_CASE("set_locales stores a single locale and get_locales returns it",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    tts_locale loc;
    loc.id       = "com.example.en-US";
    loc.name     = "English (US)";
    loc.language = "en";
    loc.country  = "US";

    // Act
    tts.set_locales({ loc });

    // Assert
    auto locales = tts.get_locales();
    REQUIRE(locales.size() == 1);
    CHECK(locales[0].id       == "com.example.en-US");
    CHECK(locales[0].name     == "English (US)");
    CHECK(locales[0].language == "en");
    CHECK(locales[0].country  == "US");
}

TEST_CASE("set_locales stores multiple locales and get_locales returns all",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    tts_locale en_us;
    en_us.id       = "en-US-id";
    en_us.name     = "English (US)";
    en_us.language = "en";
    en_us.country  = "US";

    tts_locale fr_fr;
    fr_fr.id       = "fr-FR-id";
    fr_fr.name     = "French (France)";
    fr_fr.language = "fr";
    fr_fr.country  = "FR";

    tts_locale ja_jp;
    ja_jp.id       = "ja-JP-id";
    ja_jp.name     = "Japanese (Japan)";
    ja_jp.language = "ja";
    ja_jp.country  = "JP";

    // Act
    tts.set_locales({ en_us, fr_fr, ja_jp });

    // Assert
    auto locales = tts.get_locales();
    REQUIRE(locales.size() == 3);
    CHECK(locales[0] == en_us);
    CHECK(locales[1] == fr_fr);
    CHECK(locales[2] == ja_jp);
}

TEST_CASE("set_locales replaces previously set locales",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    tts_locale first;
    first.id = "first-locale";
    tts.set_locales({ first });
    REQUIRE(tts.get_locales().size() == 1);

    tts_locale second;
    second.id = "second-locale";

    // Act
    tts.set_locales({ second });

    // Assert
    auto locales = tts.get_locales();
    REQUIRE(locales.size() == 1);
    CHECK(locales[0].id == "second-locale");
}

TEST_CASE("set_locales with empty vector clears the locale list",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    tts_locale loc;
    loc.id = "some-locale";
    tts.set_locales({ loc });
    REQUIRE_FALSE(tts.get_locales().empty());

    // Act
    tts.set_locales({});

    // Assert
    CHECK(tts.get_locales().empty());
}

// ---------------------------------------------------------------------------
// reset()
// ---------------------------------------------------------------------------

TEST_CASE("reset() clears spoken_texts, last_options, and speak_count",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    speech_options opts;
    opts.volume = 0.6;
    tts.speak("before reset");
    tts.speak("with opts", opts);
    REQUIRE(tts.speak_count() == 2);

    // Act
    tts.reset();

    // Assert
    CHECK(tts.speak_count() == 0);
    CHECK(tts.spoken_texts().empty());
    CHECK_FALSE(tts.last_options().has_value());
}

TEST_CASE("reset() does NOT clear the canned locale list",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;
    tts_locale loc;
    loc.id = "keep-me";
    tts.set_locales({ loc });
    tts.speak("something");

    // Act
    tts.reset();

    // Assert - locales survive reset
    REQUIRE(tts.get_locales().size() == 1);
    CHECK(tts.get_locales()[0].id == "keep-me");
}

TEST_CASE("reset() can be called on a pristine instance without side effects",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech tts;

    // Act
    tts.reset();

    // Assert - still in default state
    CHECK(tts.speak_count() == 0);
    CHECK(tts.spoken_texts().empty());
    CHECK_FALSE(tts.last_options().has_value());
}

// ---------------------------------------------------------------------------
// Interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("mock_text_to_speech is usable through the abstract text_to_speech pointer",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech impl;
    text_to_speech* iface = &impl;

    // Act
    iface->speak("via interface");
    speech_options opts;
    opts.volume = 0.5;
    opts.locale = "es-ES";
    iface->speak("via interface with opts", opts);

    // Assert via concrete type
    CHECK(impl.speak_count() == 2);
    REQUIRE(impl.spoken_texts().size() == 2);
    CHECK(impl.spoken_texts()[0] == "via interface");
    CHECK(impl.spoken_texts()[1] == "via interface with opts");
    REQUIRE(impl.last_options().has_value());
    CHECK(impl.last_options()->volume == 0.5);
    CHECK(impl.last_options()->locale == "es-ES");
}

TEST_CASE("get_locales is callable through the abstract interface",
          "[mock][essentials][tts]") {
    // Arrange
    mock_text_to_speech impl;
    tts_locale loc;
    loc.id = "iface-locale";
    impl.set_locales({ loc });
    text_to_speech* iface = &impl;

    // Act
    auto locales = iface->get_locales();

    // Assert
    REQUIRE(locales.size() == 1);
    CHECK(locales[0].id == "iface-locale");
}

// ---------------------------------------------------------------------------
// Value-type equality: speech_options
// ---------------------------------------------------------------------------

TEST_CASE("speech_options equality operator",
          "[mock][essentials][tts][valuetype]") {
    speech_options a;
    a.volume = 0.8;
    a.pitch  = 1.2;
    a.locale = "en-GB";

    speech_options b = a;
    CHECK(a == b);

    b.pitch = 0.9;
    CHECK_FALSE(a == b);
}

TEST_CASE("speech_options default values are volume=1.0, pitch=1.0, locale=empty",
          "[mock][essentials][tts][valuetype]") {
    speech_options opts;
    CHECK(opts.volume == 1.0);
    CHECK(opts.pitch  == 1.0);
    CHECK(opts.locale == "");
}

// ---------------------------------------------------------------------------
// Value-type equality: tts_locale
// ---------------------------------------------------------------------------

TEST_CASE("tts_locale equality operator",
          "[mock][essentials][tts][valuetype]") {
    tts_locale a;
    a.id       = "x";
    a.name     = "X";
    a.language = "xx";
    a.country  = "XX";

    tts_locale b = a;
    CHECK(a == b);

    b.country = "YY";
    CHECK_FALSE(a == b);
}

TEST_CASE("tts_locale default-constructed fields are empty strings",
          "[mock][essentials][tts][valuetype]") {
    tts_locale loc;
    CHECK(loc.id.empty());
    CHECK(loc.name.empty());
    CHECK(loc.language.empty());
    CHECK(loc.country.empty());
}
