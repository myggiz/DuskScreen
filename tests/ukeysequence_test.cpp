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

// getSimpleKeys()/getModifiers() partition mKeys (in insertion order) by
// isModifier(). Neither was previously exercised.

TEST(UKeySequenceParts, SplitsModifiersFromSimpleKeys) {
    UKeySequence seq("Ctrl+Shift+A");  // mKeys = [Control, Shift, A]
    QVector<Qt::Key> mods = seq.getModifiers();
    QVector<Qt::Key> simple = seq.getSimpleKeys();
    ASSERT_EQ(mods.size(), 2);
    EXPECT_EQ(mods[0], Qt::Key_Control);  // insertion order preserved
    EXPECT_EQ(mods[1], Qt::Key_Shift);
    ASSERT_EQ(simple.size(), 1);
    EXPECT_EQ(simple[0], Qt::Key_A);
}

TEST(UKeySequenceParts, NoModifiersYieldsEmptyModifierList) {
    UKeySequence seq("F5");
    EXPECT_TRUE(seq.getModifiers().isEmpty());
    ASSERT_EQ(seq.getSimpleKeys().size(), 1);
    EXPECT_EQ(seq.getSimpleKeys()[0], Qt::Key_F5);
}

TEST(UKeySequenceParts, ModifierOnlyYieldsEmptySimpleList) {
    UKeySequence seq("Ctrl");
    EXPECT_TRUE(seq.getSimpleKeys().isEmpty());
    ASSERT_EQ(seq.getModifiers().size(), 1);
    EXPECT_EQ(seq.getModifiers()[0], Qt::Key_Control);
}

// Builder API — addKey(Qt::Key), addKey(const QString&), addModifiers() — all
// previously reached only indirectly (or not at all).

TEST(UKeySequenceBuilder, AddKeyDeduplicates) {
    UKeySequence seq;
    seq.addKey(Qt::Key_A);
    seq.addKey(Qt::Key_A);  // duplicate ignored
    ASSERT_EQ(seq.size(), static_cast<size_t>(1));
    EXPECT_EQ(seq[0], Qt::Key_A);
}

TEST(UKeySequenceBuilder, AddKeyRejectsNonPositive) {
    UKeySequence seq;
    seq.addKey(static_cast<Qt::Key>(0));  // key <= 0 guard
    EXPECT_EQ(seq.size(), static_cast<size_t>(0));
}

TEST(UKeySequenceBuilder, AddKeyStringParsesLetter) {
    // Exercises the Qt 6-ported branch: QKeySequence(key) -> seq[0].key().
    UKeySequence seq;
    seq.addKey(QString("B"));
    ASSERT_EQ(seq.size(), static_cast<size_t>(1));
    EXPECT_EQ(seq[0], Qt::Key_B);
}

TEST(UKeySequenceBuilder, AddKeyStringRejectsCompoundToken) {
    UKeySequence seq;
    seq.addKey(QString("Ctrl+A"));  // contains '+' -> rejected whole
    EXPECT_EQ(seq.size(), static_cast<size_t>(0));
}

TEST(UKeySequenceBuilder, AddKeyStringRecognizesWinAlias) {
    UKeySequence seq;
    seq.addKey(QString("win"));  // "win"/"meta" alias -> Meta
    ASSERT_EQ(seq.size(), static_cast<size_t>(1));
    EXPECT_EQ(seq[0], Qt::Key_Meta);
}

TEST(UKeySequenceBuilder, AddKeyStringRejectsUnparsable) {
    // Distinct rejection path from the '+'/',' guard: an unparsable token yields
    // QKeySequence::count() != 1, so addKey bails without adding.
    UKeySequence seq;
    seq.addKey(QString(""));
    EXPECT_EQ(seq.size(), static_cast<size_t>(0));
}

TEST(UKeySequenceBuilder, AddModifiersExpandsShiftControl) {
    UKeySequence seq;
    seq.addKey(Qt::Key_A);
    seq.addModifiers(Qt::ControlModifier | Qt::ShiftModifier);
    // addModifiers adds in Shift, Control, Alt, Meta order -> mKeys=[A,Shift,Control]
    EXPECT_EQ(seq.toString().toStdString(), "Shift+Ctrl+A");
}

TEST(UKeySequenceBuilder, AddModifiersExpandsAltMeta) {
    UKeySequence seq;
    seq.addKey(Qt::Key_A);
    seq.addModifiers(Qt::AltModifier | Qt::MetaModifier);
    // Drives the Alt + Meta arms of addModifiers -> mKeys=[A,Alt,Meta]
    EXPECT_EQ(seq.toString().toStdString(), "Alt+Meta+A");
}

TEST(UKeySequenceBuilder, AddModifiersNoModifierPreservesState) {
    UKeySequence seq;
    seq.addKey(Qt::Key_A);
    seq.addModifiers(Qt::NoModifier);  // adds nothing, corrupts nothing
    ASSERT_EQ(seq.size(), static_cast<size_t>(1));
    EXPECT_EQ(seq[0], Qt::Key_A);
}

// keyToStr Alt/Meta arms — previously only Ctrl/Shift were covered.

TEST(UKeySequenceRoundTrip, AltFunctionKey) {
    UKeySequence seq("Alt+F1");
    EXPECT_EQ(seq.toString().toStdString(), "Alt+F1");
}

TEST(UKeySequenceRoundTrip, MetaLetter) {
    UKeySequence seq("Meta+X");
    EXPECT_EQ(seq.toString().toStdString(), "Meta+X");
}
