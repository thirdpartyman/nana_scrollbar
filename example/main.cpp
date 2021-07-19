#include <nana/gui.hpp>
#include <nana/gui/widgets/spinbox.hpp>
#include <scrollbar.hpp>

int main()
{
    using namespace nana;

    form fm{API::make_center(1000, 700)};
    fm.div("vert<topbar weight=30><<p><scrollbar weight=5%>>");

    scrollbar scroll{fm};
    fm["scrollbar"] << scroll;

    spinbox numeric{fm};
    numeric.range(0, 10000, 1); numeric.value("5000");
    fm["topbar"] << numeric;
    button btn{fm, "amount"};
    fm["topbar"] << btn;


    panel<true> p{fm};
    p.bgcolor(nana::colors::blue);
    fm["p"] << p;
    panel<true> panel(p, {0, 0, 300, 400});
    panel.bgcolor(nana::colors::red);

    std::vector<std::unique_ptr<button>> buttons;
    place plc(panel);
    plc.div("vert<vert buttons>");
    btn.events().click([&](arg_click arg){
        decltype(panel.size().width) height = numeric.to_int();
        panel.size({panel.size().width, height});
        scroll.amount(height, p.size().height);

        buttons.clear();
        size_t button_weight = 20;
        for(int i = 0; i < height / button_weight; i++)
        {
            plc["buttons"] <<*buttons.emplace_back(new button(panel, "button" + std::to_string(i)));
        }
        scroll.step(button_weight);
        plc.collocate();
    });

    p.events().resized([&](arg_resized arg){
        scroll.amount(panel.size().height, p.size().height);
    });

    scroll.events().value_changed([&](scrollbar<button>::arg_scroll arg)
    {
        panel.move(0, -arg.value);
    });


    p.events().mouse_wheel([&](arg_wheel arg){
        scroll.scroll(!arg.upwards);
    });


    auto keypress = [&](nana::arg_keyboard arg){
        bool forward;
        switch(arg.key) {
        case 33:forward = false; break;                         //PageUp
        case 34:forward = true; break;                          //PageDown
        case 65360: scroll.value(0); return;                    //Home
        case 65367: scroll.value(panel.size().height); return;  //End
        default: return;
        }
        scroll.page_scroll(forward);
    };

    fm.events().key_press(keypress);
    scroll.events().key_press(keypress);
    scroll.slider().events().key_press(keypress);

    btn.events().click.emit({}, fm);

    fm.collocate();
    fm.show();
    exec();
}
