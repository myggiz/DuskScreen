#include <gtest/gtest.h>
#include <ukeysequence.h>

// keyToStr: Control->"Ctrl", Shift->"Shift", Alt->"Alt", Meta->"Meta",
// else QKeySequence(key).toString(). Modifiers are emitted before simple keys,
// in insertion order. toString() is non-const, so the object is non-const.

TEST(UKeySequenceRoundTrip, ModifiersAndLetter) {
    UKeySequence seq("Ctrl+Shift+A");
    EXPECT_EQ(seq.toString().toStdString(), "Ctrl+Shift+A");
}

TEST(UKeySequenceRoundTrip, SingleModifierLetter) {
    UKeySequence seq("Ctrl+S");
    EXPECT_EQ(seq.toString().toStdString(), "Ctrl+S");
}

TEST(UKeySequenceRoundTrip, FunctionKey) {
    UKeySequence seq("F5");
    EXPECT_EQ(seq.toString().toStdString(), "F5");
}

TEST(UKeySequenceParse, SizeCountsEveryKey) {
    UKeySequence seq("Ctrl+Shift+A");
    EXPECT_EQ(seq.size(), static_cast<size_t>(3));
}

// Negative / edge cases — each pins a distinct parser failure mode against regression.

TEST(UKeySequenceParse, DuplicateModifierDeduplicated) {
    UKeySequence seq("Ctrl+Ctrl+A");
    EXPECT_EQ(seq.size(), static_cast<size_t>(2));
    EXPECT_EQ(seq.toString().toStdString(), "Ctrl+A");
}

TEST(UKeySequenceParse, CommaContainingTokenRejected) {
    UKeySequence seq("Ctrl,A");
    EXPECT_EQ(seq.size(), static_cast<size_t>(0));
}

TEST(UKeySequenceParse, EmptyStringYieldsNoKeys) {
    UKeySequence seq("");
    EXPECT_EQ(seq.size(), static_cast<size_t>(0));
}

// Boundary: operator[] must return Qt::Key_unknown for an out-of-range index.
// The upstream guard `(int)n > mKeys.size()` is off-by-one — at n == size() it
// falls through and reads mKeys[size()] out of bounds. Our overlay fixes it to
// `n >= size()`. Valid indices return the real keys; the one-past-end index
// returns Key_unknown (rather than crashing / reading garbage).
TEST(UKeySequenceIndex, OutOfRangeReturnsUnknown) {
    UKeySequence seq("Ctrl+A");
    ASSERT_EQ(seq.size(), static_cast<size_t>(2));
    EXPECT_EQ(seq[0], Qt::Key_Control);
    EXPECT_EQ(seq[1], Qt::Key_A);
    EXPECT_EQ(seq[seq.size()], Qt::Key_unknown);
}
