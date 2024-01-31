class Singleton {
 public:
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;
  Singleton(Singleton&&) = delete;
  Singleton& operator=(Singleton&&) = delete;

  static Singleton& get_instance() {
    static Singleton instance;
    return instance;
  }

 private:
  Singleton() = default;
};

Singleton& instance = Singleton::get_instance();
