#pragma once

#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/panel.hpp>
#include <nana/gui/dragger.hpp>

#include <algorithm>
#include <iostream>
#include <thread>
#include <stop_token>

template<class slider_class = nana::button>
class scrollbar : public nana::panel<true>
{
    using base = nana::panel<true>;
    using size_type = std::size_t;

    class slider : public slider_class
    {
        using base = nana::button;
        using base::caption;
        using base::caption_native;
        using base::caption_wstring;
        using base::size;
        using base::move;
        using base::pos;
        using base::close;
        using base::create;
        friend class scrollbar;
    public:
        slider(nana::window wd)
            : base(wd) {}

        nana::size size()
        {
            return base::size();
        }
    };

    slider _slider{ *this };
    nana::dragger _dragger;

    size_type _amount = 0;
    size_type _viewport = 0;
    size_type _step = 1;
    size_type _value = 0;

    size_type fixed_side = 0;
    //weight
    //other_side
    //pos
    //other_pos
    //make_size
    //make_pos
private:
    std::function<void(nana::arg_move)> onmove_change_value = [&](nana::arg_move arg){
        auto y = std::clamp(arg.y, 0, (int)range());
        _value = slider_pos_to_real_pos(y);
        value_changed.emit({_value}, *this);

    };
    nana::event_handle onmove_handle;

    std::jthread thread_for_mouse_down_event;

public:
    scrollbar(nana::window wd) : base(wd)
    {
        onmove_handle = _slider.events().move([&](nana::arg_move arg){
            auto y = std::clamp(arg.y, 0, (int)range());
            _slider.move(fixed_side, y);
        });
        onmove_handle = _slider.events().move(onmove_change_value);

        base::events().resized([&](nana::arg_resized arg){
            _slider.size({arg.width - 1, _slider.size().height});
            set_slider_height();
            value(_value);
        });

        _slider.events().resized([&](nana::arg_resized arg){
            nana::API::show_window(*this, arg.height != size().height);
        });

        _slider.events().mouse_wheel([&](nana::arg_wheel arg){
            step(!arg.upwards);
        });

        _slider.events().mouse_leave([&](nana::arg_mouse arg){
            focus();
        });

        //??
        _slider.events().mouse_enter([&](nana::arg_mouse arg){
            _slider.focus();
            thread_for_mouse_down_event.request_stop();
        });

        events().mouse_leave([&](nana::arg_mouse arg){
            thread_for_mouse_down_event.request_stop();
        });

        base::events().mouse_down([&](nana::arg_mouse arg){

            thread_for_mouse_down_event = std::jthread([&, pos = arg.pos.y](std::stop_token stop_token){
                //              pool.push([&, pos = arg.pos.y]{

                while (!stop_token.stop_requested()) {
                    puts("mouse_down");
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(200ms);
                    auto slider_pos = _slider.pos().y;
                    std::cout << "pos " << pos << std::endl;
                    std::cout << "slider_pos " << slider_pos << std::endl;
                    std::cout << "cursor_position " << nana::API::cursor_position().y << std::endl;
                    ;
                    if (pos < _slider.pos().y) {
                        page_scroll(false);
                        puts("page_scroll(false)");
                        continue;
                    }
                    if (pos > _slider.pos().y + _slider.size().height) {
                        step(true);
                        page_scroll("page_scroll(true)");
                        continue;
                    }
                }
            });

        });

        events().mouse_up([&](nana::arg_mouse arg){
            thread_for_mouse_down_event.request_stop();
        });


        _dragger.trigger(_slider);
        _dragger.target(_slider);
    }


    void amount(size_type amount, size_type viewport)
    {
        _amount = amount;
        _viewport = viewport;
        set_slider_height();
    }

    size_type amount()
    {
        return _amount;
    }

    size_type viewport()
    {
        return _viewport;
    }

    size_type range()
    {
        return size().height - _slider.size().height;
    }

    size_type value()
    {
        return slider_pos_to_real_pos(_slider.pos().y);
    }

    void value(size_type value)
    {
        _slider.events().move.remove(onmove_handle);
        _slider.move(fixed_side, real_pos_to_slider_pos(value));
        value_changed.emit({_value = std::clamp(value, (size_type)0, (size_type)scrollable_amount())}, *this);
        onmove_handle = _slider.events().move(onmove_change_value);
    }

    slider& slider()
    {
        return _slider;
    }

    void step(size_type step)
    {
        _step = step;
    }

    size_type step()
    {
        return _step;
    }


    void step(bool forward, size_type steps = 1)
    {
        if (forward) {
            _value += _step * steps;
            if (_value > scrollable_amount())
                _value = scrollable_amount();
        } else {
            auto _weight = _step * steps;
            _value = (_weight > _value) ? 0 : _value - _weight;
        }

        value(_value);
    }

    void scroll(bool forward)
    {
        auto pos = _slider.pos();
        pos.y += (forward) ? 1 : -1;
        _slider.move(pos);
    }

    void page_scroll(bool forward)
    {
        if (forward == false && _viewport > _value) {
            value(0);
            return;
        }
        value(forward ? _value + _viewport : _value - _viewport);
    }

private:

    std::make_signed_t<size_type> scrollable_amount()
    {
        using sig = std::make_signed_t<size_type>;
        return (sig)_amount - (sig)_viewport;
    }

    size_type min_slider_height()
    {
        return size().height / 20.;
    }

    size_type max_slider_height()
    {
        return size().height;
    }

    size_type slider_amount()
    {
        return size().height - _slider.size().height;
    }

    size_type slider_pos_to_real_pos(size_type slider_pos)
    {
        // pos / amount = slider_pos / scrollbar_range
        double scrollbar_range = range();
        double amount = scrollable_amount();
        return slider_pos / scrollbar_range * amount;
    }

    size_type real_pos_to_slider_pos(size_type pos)
    {
        // pos / amount = slider_pos / scrollbar_range
        double scrollbar_range = range();
        double amount = scrollable_amount();
        return pos * scrollbar_range / amount;
    }

    size_type slider_step()
    {
        // step / amount = slider_step / scrollbar_range
        double scrollbar_range = range();
        double amount = scrollable_amount();
        return _step * scrollbar_range / amount;
    }

private:
    void set_slider_height()
    {
        if (scrollable_amount() <= 0) {
            _slider.size(nana::size(_slider.size().width, max_slider_height()));
            return;
        }

        size_type visible_height = _viewport;
        size_type full_height = _amount;
        size_type slider_height;
        size_type scrollbar_height = size().height;

        // visible_height / full_height = slider_height / scrollbar_height
        slider_height = visible_height * scrollbar_height / (double)full_height;
        slider_height = std::clamp(slider_height, min_slider_height(), max_slider_height());
        _slider.size({_slider.size().width, (nana::size::value_type)slider_height});
    }


public:

    struct arg_scroll : public nana::event_arg
    {
        size_type value;
        arg_scroll(size_type position) : value(position) {}
    };

    nana::basic_event<arg_scroll> value_changed;

    struct Events
    {
        virtual ~Events() = default;
        nana::basic_event<nana::arg_mouse>& mouse_enter;
        nana::basic_event<nana::arg_mouse>& mouse_move;
        nana::basic_event<nana::arg_mouse>& mouse_leave;
        nana::basic_event<nana::arg_mouse>& mouse_down;
        nana::basic_event<nana::arg_mouse>& mouse_up;
        nana::basic_event<nana::arg_click>& click;
        nana::basic_event<nana::arg_mouse>& dbl_click;
        nana::basic_event<nana::arg_wheel>& mouse_wheel;
        nana::basic_event<nana::arg_dropfiles>&	mouse_dropfiles;
        nana::basic_event<nana::arg_expose>&	expose;
        nana::basic_event<nana::arg_focus>&	focus;
        nana::basic_event<nana::arg_keyboard>&	key_press;
        nana::basic_event<nana::arg_keyboard>&	key_release;
        nana::basic_event<nana::arg_keyboard>&	key_char;
        nana::basic_event<nana::arg_keyboard>&	shortkey;
        nana::basic_event<nana::arg_move>&		move;
        nana::basic_event<nana::arg_resizing>&	resizing;
        nana::basic_event<nana::arg_resized>&	resized;
        nana::basic_event<nana::arg_destroy>&	destroy;

        nana::basic_event<arg_scroll>& value_changed;


        Events(event_type& events, nana::basic_event<arg_scroll>& value_changed) : mouse_enter(events.mouse_enter), mouse_move(events.mouse_move), mouse_leave(events.mouse_leave), mouse_down(events.mouse_down), mouse_up(events.mouse_up), click(events.click), dbl_click(events.dbl_click), mouse_wheel(events.mouse_wheel), mouse_dropfiles(events.mouse_dropfiles), expose(events.expose), focus(events.focus), key_press(events.key_press), key_release(events.key_release), key_char(events.key_char), shortkey(events.shortkey), move(events.move), resizing(events.resizing), resized(events.resized), destroy(events.destroy), value_changed(value_changed)
        {}
    };

    Events events()
    {
        return { base::events(), value_changed };
    }
};
