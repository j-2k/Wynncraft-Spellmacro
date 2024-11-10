#include <iostream>
#include <windows.h>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

std::queue<char> keyQueue; // Queue to store keys
std::mutex mtx;             // Mutex for thread-safe access to the queue
std::condition_variable cv; // Condition variable for managing the queue

// Function to simulate a mouse click
void SimulateMouseClick(int button) {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = (button == 0) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Slight delay

    input.mi.dwFlags = (button == 0) ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
    SendInput(1, &input, sizeof(INPUT));
}

// Function to handle the R sequence (RRR)
void HandleRSequence() {
    std::cout << "Starting R Sequence (RRR)\n";
    SimulateMouseClick(1); // Right click
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    SimulateMouseClick(1); // Right click
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    SimulateMouseClick(1); // Right click
    std::cout << "Finished R Sequence (RRR)\n";
}

// Function to handle the Q sequence (RLR)
void HandleQSequence() {
    std::cout << "Starting Q Sequence (RLR)\n";
    SimulateMouseClick(1); // Right click
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    SimulateMouseClick(0); // Left click
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    SimulateMouseClick(1); // Right click
    std::cout << "Finished Q Sequence (RLR)\n";
}

// Function that processes keys from the queue
void ProcessKeyQueue() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return !keyQueue.empty(); }); // Wait until there's a key in the queue

        char key = keyQueue.front(); // Get the next key in the queue
        keyQueue.pop();
        lock.unlock();

        // Handle the sequence based on the key
        if (key == 'R') {
            HandleRSequence();
        }
        else if (key == 'Q') {
            HandleQSequence();
        }
    }
}

// Function to monitor key presses
void KeyPressListener() {
    bool RPressed = false;
    bool QPressed = false;

    while (true) {
        if (GetAsyncKeyState('R') & 0x8000) { // Key R down
            if (!RPressed) { // Only trigger if it wasn't already pressed
                std::lock_guard<std::mutex> lock(mtx);
                keyQueue.push('R'); // Add 'R' to the queue
                cv.notify_one();    // Notify the processing thread
                std::cout << "Key 'R' pressed, added to queue\n";
                RPressed = true;    // Mark R as pressed
            }
        }
        else {
            RPressed = false; // Reset when the key is released
        }

        if (GetAsyncKeyState('Q') & 0x8000) { // Key Q down
            if (!QPressed) { // Only trigger if it wasn't already pressed
                std::lock_guard<std::mutex> lock(mtx);
                keyQueue.push('Q'); // Add 'Q' to the queue
                cv.notify_one();    // Notify the processing thread
                std::cout << "Key 'Q' pressed, added to queue\n";
                QPressed = true;    // Mark Q as pressed
            }
        }
        else {
            QPressed = false; // Reset when the key is released
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Small delay to avoid CPU overload
    }
}

int main() {
    std::cout << "Press 'R' for RRR (right click sequence), 'Q' for RLR (right-left-right sequence).\n";

    std::thread processor(ProcessKeyQueue); // Start the queue processing thread
    std::thread listener(KeyPressListener); // Start the key listener thread

    processor.join();
    listener.join();

    return 0;
}
