#include "photo_deck.h"

#include <gtest/gtest.h>

#include <algorithm>

namespace fotoframe {
namespace {

using Paths = std::vector<std::filesystem::path>;

TEST(PhotoDeckTest, EmptyDeckGivesNothing) {
    PhotoDeck deck(1);
    EXPECT_FALSE(deck.next().has_value());
}

TEST(PhotoDeckTest, SinglePhotoIsDrawnEveryTime) {
    PhotoDeck deck(1);
    deck.update({"a.jpg"});
    for (int i = 0; i < 3; ++i) EXPECT_EQ(deck.next(), "a.jpg");
}

// n 回引いた結果を返す。
Paths draw(PhotoDeck& deck, int n) {
    Paths drawn;
    for (int i = 0; i < n; ++i) drawn.push_back(*deck.next());
    return drawn;
}

Paths sorted(Paths paths) {
    std::ranges::sort(paths);
    return paths;
}

const Paths kFive = {"a.jpg", "b.jpg", "c.jpg", "d.jpg", "e.jpg"};

TEST(PhotoDeckTest, EachPhotoOnceInOneRound) {
    PhotoDeck deck(1);
    deck.update(kFive);
    EXPECT_EQ(sorted(draw(deck, 5)), kFive);
}

TEST(PhotoDeckTest, EachPhotoOnceInFollowingRounds) {
    PhotoDeck deck(1);
    deck.update(kFive);
    for (int round = 0; round < 10; ++round) EXPECT_EQ(sorted(draw(deck, 5)), kFive);
}

TEST(PhotoDeckTest, NoSamePhotoTwiceInARowAcrossRounds) {
    PhotoDeck deck(1);
    deck.update({"a.jpg", "b.jpg"});
    const Paths drawn = draw(deck, 100);
    for (size_t i = 1; i < drawn.size(); ++i) EXPECT_NE(drawn[i - 1], drawn[i]) << "at " << i;
}

TEST(PhotoDeckTest, UpdateWithSameListKeepsCurrentRound) {
    PhotoDeck deck(1);
    deck.update(kFive);
    Paths drawn = draw(deck, 2);
    deck.update(kFive);
    const Paths rest = draw(deck, 3);
    drawn.insert(drawn.end(), rest.begin(), rest.end());
    EXPECT_EQ(sorted(drawn), kFive);
}

TEST(PhotoDeckTest, AddedPhotoAppearsInCurrentRound) {
    PhotoDeck deck(1);
    deck.update(kFive);
    Paths drawn = draw(deck, 2);
    Paths six = kFive;
    six.push_back("f.jpg");
    deck.update(six);
    const Paths rest = draw(deck, 4);
    drawn.insert(drawn.end(), rest.begin(), rest.end());
    EXPECT_EQ(sorted(drawn), six);
}

TEST(PhotoDeckTest, RemovedPhotoIsNeverDrawn) {
    PhotoDeck deck(1);
    deck.update(kFive);
    draw(deck, 2);
    deck.update({"a.jpg", "b.jpg", "d.jpg", "e.jpg"});
    for (const auto& photo : draw(deck, 40)) EXPECT_NE(photo, "c.jpg");
}

TEST(PhotoDeckTest, UpdateToEmptyGivesNothing) {
    PhotoDeck deck(1);
    deck.update(kFive);
    draw(deck, 2);
    deck.update({});
    EXPECT_FALSE(deck.next().has_value());
}

}  // namespace
}  // namespace fotoframe
