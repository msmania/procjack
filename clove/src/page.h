struct CodeTemplate;

class ExecutablePages final {
  struct Slot final {
    Slot *next_;
    uint8_t *start_;
    Slot(Slot *next, uint8_t *start);
    ~Slot();
  };
  struct ExecutablePage final {
    ExecutablePage *next_;
    uint32_t capacity_;
    uint8_t *base_;
    uint8_t *empty_head_;
    std::unique_ptr<Slot> active_slots_;
    ExecutablePage(uint32_t capacity, void *buffer);
    ~ExecutablePage();
    void *Push(const CodeTemplate &code);
    const void *TryPush(const CodeTemplate &code) const;
    bool Revert(const void *last_pushed);
  };

  std::unique_ptr<ExecutablePage> active_head_;

public:
  void *Push(const CodeTemplate &code, const void *source);
  bool Revert(const void *last_pushed);
};
