// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include "common/event.h"

using namespace gcs::common;

TEST(SignalTest, ConnectAndInvoke) {
    Signal<int> signal;
    int received_value = 0;

    auto connection = signal.Connect([&](int value) {
        received_value = value;
    });

    signal.Invoke(42);
    EXPECT_EQ(received_value, 42);
}

TEST(SignalTest, MultipleListeners) {
    Signal<int> signal;
    int count = 0;

    auto conn1 = signal.Connect([&](int) { count++; });
    auto conn2 = signal.Connect([&](int) { count++; });

    signal.Invoke(0);
    EXPECT_EQ(count, 2);
}

TEST(SignalTest, ScopedConnection) {
    Signal<int> signal;
    int count = 0;

    {
        auto conn = signal.Connect([&](int) { count++; });
        signal.Invoke(0);
        EXPECT_EQ(count, 1);
    } // conn goes out of scope and disconnects

    signal.Invoke(0);
    EXPECT_EQ(count, 1); // count should still be 1
}

TEST(SignalTest, ThreadSafety) {
    Signal<int> signal;
    std::atomic<int> count = 0;
    const int num_threads = 10;
    const int invites_per_thread = 100;

    auto conn = signal.Connect([&](int) { count++; });

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < invites_per_thread; ++j) {
                signal.Invoke(0);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(count, num_threads * invites_per_thread);
}
