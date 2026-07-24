#include <gtest/gtest.h>
#include <tools/screenshot.h>

// The Screenshot constructor forces quality to 80 for PNG (where the numeric
// "quality" knob is meaningless) and leaves it untouched for lossy formats.
// Reached via the public constructor + options() accessor — no display needed.

static Screenshot::Options optsWith(Screenshot::Format format, int quality) {
    Screenshot::Options o{};
    o.format = format;
    o.quality = quality;
    return o;
}

TEST(ScreenshotOptions, PngForcesQualityEighty) {
    Screenshot s(nullptr, optsWith(Screenshot::PNG, 10));
    EXPECT_EQ(s.options().quality, 80);           // overridden
    EXPECT_EQ(s.options().format, Screenshot::PNG);
}

TEST(ScreenshotOptions, JpegPreservesQuality) {
    Screenshot s(nullptr, optsWith(Screenshot::JPEG, 55));
    EXPECT_EQ(s.options().quality, 55);           // untouched
    EXPECT_EQ(s.options().format, Screenshot::JPEG);
}

// BMP has a meaningless quality knob (like PNG) yet the ctor does NOT force it —
// only PNG is special-cased. This pins that intentional asymmetry on a distinct
// enum value rather than duplicating the JPEG pass-through branch.
TEST(ScreenshotOptions, BmpPreservesQuality) {
    Screenshot s(nullptr, optsWith(Screenshot::BMP, 42));
    EXPECT_EQ(s.options().quality, 42);
}
