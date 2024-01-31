class Singleton {
 public:
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;
  Singleton(Singleton&&) = delete;
  Singleton& operator=(Singleton&&) = delete;

  static Singleton& get_instance() {
    /* local static */
    static Singleton instance;
    return instance;
  }

 private:
  Singleton() = default;
};
