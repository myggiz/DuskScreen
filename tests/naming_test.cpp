#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QRegularExpression>
#include <tools/screenshot.h>

static Screenshot::NamingOptions opt(Screenshot::Naming n, bool flip = false,
                                     int lz = 0, const QString &df = QString())
{
    return Screenshot::NamingOptions{n, flip, lz, df};
}

TEST(GetName, NumericEmptyDir) {
    QTemporaryDir dir; ASSERT_TRUE(dir.isValid());
    QString r = Screenshot::getName(opt(Screenshot::Numeric), "shot", QDir(dir.path()));
    EXPECT_EQ(r.toStdString(), "shot1");
}

TEST(GetName, NumericLeadingZeros) {
    QTemporaryDir dir; ASSERT_TRUE(dir.isValid());
    QString r = Screenshot::getName(opt(Screenshot::Numeric, false, 3), "shot", QDir(dir.path()));
    EXPECT_EQ(r.toStdString(), "shot0001");
}

TEST(GetName, NumericFlipPutsCounterFirst) {
    QTemporaryDir dir; ASSERT_TRUE(dir.isValid());
    QString r = Screenshot::getName(opt(Screenshot::Numeric, true), "shot", QDir(dir.path()));
    EXPECT_EQ(r.toStdString(), "1shot");
}

TEST(GetName, NumericIncrementsPastLargestExisting) {
    QTemporaryDir dir; ASSERT_TRUE(dir.isValid());
    for (const char *n : {"shot3.png", "shot7.png"}) {
        QFile f(QDir(dir.path()).filePath(n));
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.close();
    }
    QString r = Screenshot::getName(opt(Screenshot::Numeric), "shot", QDir(dir.path()));
    EXPECT_EQ(r.toStdString(), "shot8");
}

// Negative: the scan only counts files whose name contains the prefix — a
// non-matching file must not advance the counter. Guards the `file.contains(prefix)` filter.
TEST(GetName, NumericIgnoresFilesWithoutPrefix) {
    QTemporaryDir dir; ASSERT_TRUE(dir.isValid());
    QFile f(QDir(dir.path()).filePath("other5.png"));
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();
    QString r = Screenshot::getName(opt(Screenshot::Numeric), "shot", QDir(dir.path()));
    EXPECT_EQ(r.toStdString(), "shot1");
}

// Boundary: existing largest = 9, leadingZeros = 0 -> counter rolls to two digits.
// Guards the `largest + 1` increment width transition.
TEST(GetName, NumericCounterRollsOverToTwoDigits) {
    QTemporaryDir dir; ASSERT_TRUE(dir.isValid());
    QFile f(QDir(dir.path()).filePath("shot9.png"));
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();
    QString r = Screenshot::getName(opt(Screenshot::Numeric), "shot", QDir(dir.path()));
    EXPECT_EQ(r.toStdString(), "shot10");
}

TEST(GetName, EmptyNamingIsPrefixOnly) {
    QTemporaryDir dir; ASSERT_TRUE(dir.isValid());
    QString r = Screenshot::getName(opt(Screenshot::Empty), "shot", QDir(dir.path()));
    EXPECT_EQ(r.toStdString(), "shot");
}

TEST(GetName, TimestampShape) {
    QTemporaryDir dir; ASSERT_TRUE(dir.isValid());
    QString r = Screenshot::getName(opt(Screenshot::Timestamp), "shot", QDir(dir.path()));
    EXPECT_TRUE(QRegularExpression("^shot\\d+$").match(r).hasMatch()) << r.toStdString();
}

TEST(GetName, DateYearShape) {
    QLocale::setDefault(QLocale::c());
    QTemporaryDir dir; ASSERT_TRUE(dir.isValid());
    QString r = Screenshot::getName(opt(Screenshot::Date, false, 0, "yyyy"), "shot", QDir(dir.path()));
    EXPECT_TRUE(QRegularExpression("^shot\\d{4}$").match(r).hasMatch()) << r.toStdString();
}
