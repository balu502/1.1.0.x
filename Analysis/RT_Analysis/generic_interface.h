class External_Interface
{
public:
    virtual void __stdcall destroy() = 0;
    virtual int __stdcall do_stuff(int param) = 0;
    virtual void __stdcall do_something_else(double f) = 0;
    virtual void __stdcall load_protocol(char*) = 0;

    virtual HWND __stdcall create_win(HWND) = 0;
    virtual void __stdcall destroy_win() = 0;
    virtual void __stdcall show() = 0;
};

