#include "signals.hpp"
#include <iostream>

// Delegate that holds function callbacks.
// For simplicity, delegates are limited to void return types.
template<typename... Args>
class Delegate
{
public:
    using signal = sig::signal<void(Args...)>;
    using conection = typename signal::connection_type;

    // Adds a function callback to the delegate.
    template<typename Func>
    connection operator+=(Func&& f)
    {
        return m_Callbacks.connect(std::forward<Func>(f));
    }

    // Remove a function callback.
    // Returns the number of functions removed.
    template<typename Func>
    std::size_t operator-=(Func&& f)
    {
        return m_Callbacks.disconnect(std::forward<Func>(f));
    }

    // Invoke the delegate.
    // All connected callbacks are invoked.
    void operator()(Args&&... args)
    {
        m_Callbacks(std::forward<Args>(args)...):
    }
private:
    signal m_Callbacks;
};

// Base class for all event args.
class EventArgs
{
public:
    EventArgs() = default;
    virtual ~EventArgs() = default;
};

// Define an event that takes a reference to EventArgs 
// as it's only argument.
using Event = Delegate<EventArgs&>;

class MouseMotionEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    MouseMotionEventArgs( int x, int y )
        : base()
        , X(x)
        , Y(y)
    {}

    int X;
    int Y;
};

using MouseMotionEvent = Delegate<MouseMotionEventArgs&>;

// Declare the windows processor function.
void WndProc();

// The Application class defines a few events that can be handled by 
// callback functions elsewhere.
class Application
{
public:
    Application() = default;
    virtual ~Application() = default;

    // Application events.
    Event            Update;
    Event            Render;
    MouseMotionEvent MouseMoved;

protected:
    friend void WndProc();

    virtual void OnUpdate()
    {
        EventArgs e;
        // Invoke event.
        Update(e);
    }

    virtual void OnRender()
    {
        EventArgs e;
        // Invoke event.
        Render(e);
    }

    virtual void OnMouseMoved(int x, int y)
    {
        MouseMotionEventArgs e(x, y);
        // Invoke event.
        MouseMoved(e);
    }
};

// Define a few callback functions that will handle events from the application.
// Any number of callback functions can handle the events from the application class.
void OnUpdate(EventArgs& e)
{
    std::cout << "Update game..." << std::endl;
}

void OnRender(EventArgs& e)
{
    std::cout << "Render game..." << std::endl;
}

void OnMouseMoved(MouseMotionEventArgs& e)
{
    std::cout << "Mouse moved: " << e.X << ", " << e.Y << std::endl;
}


// I know, globals are evil, but it makes the 
// example easier.
static Application app;

int main()
{
    // Register some callback functions.
    app.Update += &OnUpdate;
    app.Render += &OnRender;
    app.MouseMoved += &OnMouseMoved;

    // Execute the windows event processor
    WndProc();

    return 0;
}

// Simulate a windows process function.
void WndProc()
{
    app.OnUpdate();
    app.OnRender();
    app.OnMouseMoved(60, 80);
}