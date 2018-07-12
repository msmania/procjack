class Event {
  const HANDLE h_;
  const bool was_newly_created_;

public:
  Event(BOOL manualReset, BOOL true_to_set_signaled_initially, LPCWSTR name);
  ~Event();
  bool WasNewlyCreated() const;
  BOOL Signal() const;
  BOOL Reset() const;
  DWORD Wait(DWORD timeout_in_ms) const;
};
