typedef void (*TimeFn)(const size_t replicas);

void TestStatic(const size_t replicas);
void TestAlloc(const size_t replicas);
void TestPool(const size_t replicas);
void TestArray(const size_t replicas);
void TestFreeArray(const size_t replicas);
