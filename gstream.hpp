/**\
**
**  Written by Manuel Fischer
**
\**/

#pragma once

#include <type_traits>
#include <algorithm>
#include <utility>

    template<typename GeneratorFunc>
class GSGenerator
{
public:
    GeneratorFunc generate;
};

    template<typename ProcessorFunc>
class GSProcessor
{
public:
    ProcessorFunc process;
};

    template<typename AcceptorFunc>
class GSAcceptor
{
public:
    AcceptorFunc accept;
};



    template<typename GeneratorFunc>
auto gsGenerate(GeneratorFunc func) -> GSGenerator<GeneratorFunc>
{
    return { std::move(func) };
}

    template<typename Container>
auto gsYieldFrom(Container& container)
{
    return gsGenerate(
        [p_container = std::addressof(container)](auto yield)
        {
            for(auto& e : *p_container) yield(e);
        }
    );
}

    template<typename Container>
auto gsYieldFromCopy(Container container)
{
    return gsGenerate(
        [container = std::move(container)](auto yield)
        {
            for(auto& e : container) yield(e);
        }
    );
}


    template<typename Iterator>
auto gsYieldFrom(Iterator begin, Iterator end)
{
    return gsGenerate(
        [begin, end](auto yield)
        {
            std::for_each(begin, end, yield);
        }
    );
}





    template<typename ProcessorFunc>
GSProcessor<ProcessorFunc> gsProcess(ProcessorFunc func)
{
    return { std::move(func) };
}

    template<typename MapFunc>
auto gsMap(MapFunc&& func)
{
    return gsProcess(
        [func = std::forward<MapFunc>(func)]<typename Value, typename Yield>(Value&& value, Yield yield)
        {
            yield(func(std::forward<Value>(value)));
        }
    );
}

    template<typename FilterFunc>
auto gsFilter(FilterFunc&& func)
{
    return gsProcess(
        [func = std::forward<FilterFunc>(func)]<typename Value, typename Yield>(Value&& value, Yield yield)
        {
            if(func(value))
                yield(std::forward<Value>(value));
        }
    );
}






    template<typename AcceptorFunc>
auto gsAccept(AcceptorFunc func) -> GSAcceptor<AcceptorFunc>
{
    return { std::move(func) };
}



    template<typename Container>
auto gsInsertBack(Container& container)
{
    return gsAccept(
        [p_container = std::addressof(container)]<typename Value>(Value&& value)
        {
            p_container->push_back(std::forward<Value>(value));
        }
    );
}







    template<typename GeneratorFuncL, typename AcceptorFuncR>
void operator|(GSGenerator<GeneratorFuncL> lhs, GSAcceptor<AcceptorFuncR> rhs)
{
    lhs.generate(
        [&]<typename Value>(Value&& value)
        {
            rhs.accept(std::forward<Value>(value));
        }
    );
}


    template<typename GeneratorFuncL, typename ProcessorFuncR>
auto operator|(GSGenerator<GeneratorFuncL> lhs, GSProcessor<ProcessorFuncR> rhs)
{
    return gsGenerate(
        [lhs = std::move(lhs), rhs = std::move(rhs)](auto yield)
        {
            lhs.generate(
                [&]<typename Value>(Value&& value)
                {
                    rhs.process(std::forward<Value>(value), yield);
                }
            );
        }
    );
}



    template<typename ProcessorFuncL, typename ProcessorFuncR>
auto operator|(GSProcessor<ProcessorFuncL> lhs, GSProcessor<ProcessorFuncR> rhs)
{
    return gsProcess(
        [lhs = std::move(lhs), rhs = std::move(rhs)]<typename Value, typename Yield>(Value&& value, Yield yield)
        {
            lhs.process(std::forward<Value>(value),
                [&]<typename Value2>(Value2&& value2)
                {
                    rhs.process(std::forward<Value2>(value2), yield);
                }
            );
        }
    );
}

   template<typename ProcessorFuncL, typename AcceptorFuncR>
auto operator|(GSProcessor<ProcessorFuncL> lhs, GSAcceptor<AcceptorFuncR> rhs)
{
    return gsAccept(
        [lhs = std::move(lhs), rhs = std::move(rhs)]<typename Value>(Value&& value)
        {
            lhs.process(std::forward<Value>(value),
                [&]<typename Value2>(Value2&& value2)
                {
                    rhs.accept(std::forward<Value2>(value2));
                }
            );
        }
    );
}


    template<typename GeneratorFunc>
auto gsRef(GSGenerator<GeneratorFunc>& gen)
{
    return gsGenerate(
        [p_generate = std::addressof(gen.generate)](auto yield)
        {
            (*p_generate)(yield);
        }
    );
}

    template<typename ProcessorFunc>
auto gsRef(GSProcessor<ProcessorFunc>& prc)
{
    return gsProcess(
        [p_process = std::addressof(prc.process)]<typename Value, typename Yield>(Value&& value, Yield yield)
        {
            (*p_process)(std::forward<Value>(value), yield);
        }
    );
}

    template<typename AcceptorFunc>
auto gsRef(GSAcceptor<AcceptorFunc>& act)
{
    return gsAccept(
        [p_accept = std::addressof(act.accept)]<typename Value>(Value&& value)
        {
            (*p_accept)(std::forward<Value>(value));
        }
    );
}



namespace gsExperimental
{
        template<typename Function>
    struct GSNoCopyFunc
    {
        Function f;

        GSNoCopyFunc(GSNoCopyFunc&& f) = default;
        GSNoCopyFunc(GSNoCopyFunc const& f) = delete;

            template<typename... Args>
        auto operator()(Args&&... args) -> decltype(auto)
        {
            return f(std::forward<Args>(args)...);
        }
    };

        template<typename Function>
    auto gsNoCopyFunc(Function function) -> GSNoCopyFunc<Function>
    {
        return { std::move(function) };
    }

        template<class Iterator>
    auto gsOverwriteForward(Iterator iterator)
    {
        return gsAccept(gsNoCopyFunc(
            [iterator]<typename Value>(Value&& value) mutable
            {
                *iterator++ = std::forward<Value>(value);
            }
        ));
    }
}
