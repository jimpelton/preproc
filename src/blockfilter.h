//
// Created by jim on 4/10/16.
//

#ifndef blockfilter_h__
#define blockfilter_h__

class BlockFilter
{
public:

  void operator()();


private:

  void do_filter();



};

//template<typename Ty>
void
BlockFilter::operator()()
{
  do_filter();
}

//template<typename Ty>
void
BlockFilter::do_filter()
{

}


#endif //blockfilter_h__
