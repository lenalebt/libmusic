namespace music
{
    template <typename T1, typename T2 = T1, typename T3 = T2>
    class triple
    {
    private:
        
    protected:
        
    public:
        triple(const T1& value1, const T2& value2, const T3& value3) :
            value1(value1), value2(value2), value3(value3)
        {
            
        }
        T1 value1;
        T2 value2;
        T3 value3;
    };
    
    template <typename T1, typename T2 = T1, typename T3 = T2, typename T4 = T3>
    class quadruple
    {
    private:
        
    protected:
        
    public:
        quadruple(const T1& value1, const T2& value2, const T3& value3, const T4& value4) :
            value1(value1), value2(value2), value3(value3), value4(value4)
        {
            
        }
        T1 value1;
        T2 value2;
        T3 value3;
        T4 value4;
    };
}
