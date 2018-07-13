class Event {
  HANDLE h_;

public:
  Event();
  ~Event();
  bool CreateIfNotCreatedYet(BOOL manualReset,
                             BOOL true_to_set_signaled_initially,
                             LPCWSTR name);
  BOOL Signal() const;
  BOOL Reset() const;
  DWORD Wait(DWORD timeout_in_ms) const;
};
