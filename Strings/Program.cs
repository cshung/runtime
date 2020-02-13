using System;

namespace Strings
{
    class Holder
    {
        public string s1;
        public string s2;
        public string s3;
        public string s4;
        public string s5;
    }
    class Program
    {
        static void Main(string[] args)
        {
            Holder holder1 = new Holder();
            holder1.s1 = new string('A', 90000);
            holder1.s2 = new string('A', 90000);
            GC.Collect();
            holder1.s3 = new string('B', 90000);
            holder1.s4 = new string('B', 90000);
            Holder holder2 = new Holder();
            holder2.s1 = holder1.s1;
            holder2.s2 = holder1.s2;
            holder2.s3 = holder1.s3;
            holder2.s4 = holder1.s4;
            GC.Collect();
            GC.AddMemoryPressure(10086);
            bool win1 =  object.ReferenceEquals(holder1.s1, holder1.s2);
            bool win2 = !object.ReferenceEquals(holder1.s3, holder1.s4);
            bool win3 = !object.ReferenceEquals(holder2.s1, holder2.s2);
            bool win4 = !object.ReferenceEquals(holder2.s3, holder2.s4);
            if (win1)
            {
                Console.WriteLine("I win 1 :)");
            }
            if (win2)
            {
                Console.WriteLine("I win 2 :)");
            }
            if (win3)
            {
                Console.WriteLine("I win 3 :)");
            }
            if (win4)
            {
                Console.WriteLine("I win 4 :)");
            }
        }
    }
}
