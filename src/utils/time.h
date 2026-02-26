namespace Time {
    float GetDeltaTime(std::chrono::steady_clock::time_point now, std::chrono::steady_clock::time_point* last) {
        std::chrono::duration<float> elapsed = now - *last;
        *last = now;

        float deltaTime = elapsed.count();
        if (deltaTime > 0.1f) deltaTime = 1.0f / 60.0f;

        return deltaTime;
    }
}