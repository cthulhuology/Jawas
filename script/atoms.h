// atoms.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __ATOMS_H__
#define __ATOMS_H__

#define atom(x,y,z) const JData x = { y, #z };

// classes
atom(Array,5,Array)
atom(Boolean,7,Boolean)
atom(Date,4,Date)
atom(Error,5,Error)
atom(File,4,File)
atom(Function,8,Function)
atom(Global,6,Global)
atom(Integer,7,Integer)
atom(Math,4,Math)
atom(Number,6,Number)
atom(Object,6,Object)
atom(RegExp,6,RegExp)
atom(Socket,6,Socket)
atom(String,6,String)
atom(XML,3,XML)

// keywords
atom(Break,5,break)
atom(Case,4,case)
atom(Catch,5,catch)
atom(Continue,8,continue)
atom(Default,7,default)
atom(Delete,6,delete)
atom(Do,2,do)
atom(Else,4,else)
atom(Finally,7,finally)
atom(For,3,for)
atom(Function,8,function)
atom(If,2,if)
atom(In,2,in)
atom(Instanceof,10,instanceof)
atom(Int,3,int)
atom(New,3,new)
atom(Return,6,return)
atom(Switch,6,switch)
atom(This,4,this)
atom(Throw,5,throw)
atom(Try,3,try)
atom(Typeof,6,typeof)
atom(Var,3,var)
atom(Void,4,void)
atom(While,5,while)
atom(With,4,with)

// punctuators
atom(open_block,1,{)
atom(close_block,1,})
atom(open_paren,1,()
atom(close_paren,1,))
atom(open_bracket,1,[)
atom(close_bracket,1,))
atom(dot,1,.)
atom(semi,1,;)
atom(comma,1,,)
atom(less,1,<)
atom(greater,1,>)
atom(lesseq,2,<=)
atom(greatereq,2,>=)
atom(eqeq,2,==)
atom(noteq,2,!=)
atom(eqeqeq,3,===)
atom(noteqeq,3,!==)
atom(plus,1,+)
atom(minus,1,-)
atom(star,1,*)
atom(mod,1,%)
atom(plusplus,2,++)
atom(minusminus,2,--)
atom(lessless,2,<<)
atom(greatergreater,2,>>)
atom(greatergreatergreater,3,>>>)
atom(amp,1,&)
atom(pipe,1,|)
atom(carot,1,^)
atom(not,1,!)
atom(tilde,1,~)
atom(ampamp,2,&&)
atom(pipepipe,2,||)
atom(question,1,?)
atom(colon,1,:)
atom(eq,1,=)
atom(pluseq,2,+=)
atom(minuseq,2,-=)
atom(stareq,2,*=)
atom(modeq,2,%=)
atom(lesslesseq,3,<<=)
atom(greatergreatereq,3,>>=)
atom(greatergreatergreatereq,4,>>>=)
atom(ampeq,2,&=)
atom(pipeeq,2,|=)
atom(caroteq,2,^=)
atom(slasheq,2,/=)
atom(slash,2,/)

// literals
atom(NaN,3,NaN)
atom(Null,4,null)
atom(Undefined,9,undefined)
atom(Infinity,8,Infinity)
atom(True,4,true)
atom(False,5,false)
atom(Zero,1,0)
atom(One,1,1)
atom(NegOne,2,-1)
atom(Empty,5,empty)

// properties
atom(Construct,4,constructor)
atom(Prototype,9,prototype)
atom(Class,5,class)
atom(Get,3,get)
atom(Put,3,put)
atom(CanPut,6,canput)
atom(HasProperty,11,hasproperty)
atom(Delete,6,delete)
atom(DefaultValue,12,defaultvalue)
atom(Value,5,value)
atom(HasInstance,11,hasinstance)
atom(Scope,5,scope)
atom(Match,5,match)
atom(Call,4,call)
atom(Args,4,args)
atom(Callee,6,callee)
atom(Length,6,length)
atom(Instance,8,Instance)
atom(Enumerate,9,Enumerate)

// functions
atom(toString,8,toString)
atom(toBoolean,9,toBoolean)
atom(toInteger,9,toInteger)
atom(toNumber,8,toNumber)
atom(Push,4,push)
atom(Pop,3,pop)
atom(Shift,5,shift)
atom(Unshift,7,unshift)
atom(Eval,4,eval)
atom(isNaN,5,isNaN)
atom(decodeURI,9,decodeURI)
atom(encodeURI,9,encodeURI)
atom(valueOf,7,valueOf)
atom(hasOwnProperty,14,hasOwnProperty)
atom(isPrototypeOf,13,isPrototypeOf)
atom(propertyIsEnum,20,propertyIsEnumerable)
atom(Apply,5,apply)
atom(Concat,6,concat)
atom(Join,4,join)
atom(Reverse,7,reverse)
atom(Slice,5,slice)
atom(Sort,4,sort)
atom(Splice,6,splice)
atom(charAt,6,charAt)
atom(charCodeAt,10,charCodeAt)
atom(indexOf,7,indexOf)
atom(lastIndexOf,11,lastIndexOf)
atom(Replace,7,replace)
atom(Search,6,search)
atom(Split,5,split)
atom(SubString,9,substring)
atom(toLowerCase,11,toLowerCase)
atom(toUpperCase,11,toUpperCase)
atom(toFixed,7,toFixed)
atom(toExponential,13,toExponential)
atom(toPrecision,11,toPrecision)
atom(_E_,1,E)
atom(LN10,4,LN10)
atom(LN2,3,LN2)
atom(LOG2E,5,LOG2E)
atom(LOG10E,6,LOG10E)
atom(PI,2,PI)
atom(SQRT1_2,7,SQRT1_2)
atom(SQRT2,5,SQRT2)
atom(Abs,3,abs)
atom(Acos,4,acos)
atom(Asin,4,asin)
atom(Atan,4,atan)
atom(Atan2,5,atan2)
atom(Ceil,4,ceil)
atom(Cos,3,cos)
atom(Exp,3,exp)
atom(Floor,5,floor)
atom(Log,3,log)
atom(Max,3,max)
atom(Min,3,min)
atom(Pow,3,pow)
atom(Random,6,random)
atom(Round,5,round)
atom(Sin,3,sin)
atom(Sqrt,4,sqrt)
atom(Tan,3,tan)
atom(MakeDay,7,MakeDay)
atom(MakeTime,8,MakeTime)
atom(MakeDate,8,MakeDate)
atom(TimeClip,8,TimeClip)
atom(Parse,5,parse)
atom(UTC,3,UTC)
atom(toDateString,12,toDateString)
atom(toTimeString,12,toTimeString)
atom(getTime,7,getTime)
atom(getFullYear,11,getFullYear)
atom(getMonth,8,getMonth)
atom(getDate,7,getDate)
atom(getDay,6,getDay)
atom(getHours,8,getHours)
atom(getMinutes,10,getMinutes)
atom(getSeconds,10,getSeconds)
atom(getMilliseconds,15,getMilliseconds)
atom(getTimezone,17,getTimezoneOffset)
atom(setTime,7,setTime)
atom(setMilliseoncds,15,setMilliseconds)
atom(setSeconds,10,setSeconds)
atom(setMinutes,10,setMinutes)
atom(setHours,8,setHours)
atom(setDate,7,setDate)
atom(setMonth,8,setMonth)
atom(setFullYear,11,setFullYear)
atom(global,6,global)
atom(source,6,source)
atom(ignoreCase,10,ignoreCase)
atom(multiline,9,multiline)
atom(lastIndex,9,lastIndex)
atom(Exec,4,exec)
atom(Test,4,test)
atom(Message,7,message)
atom(Name,4,name)
atom(EvalError,9,EvalError)
atom(RangeError,10,RangeError)
atom(ReferenceError,14,ReferenceError)
atom(SyntaxError,11,SyntaxError)
atom(TypeError,9,TypeError)
atom(URIError,8,URIError)
atom(Open,4,open)
atom(Close,5,close)
atom(Read,4,read)
atom(Write,4,write)
atom(Seek,4,seek)
atom(Connect,7,connect)
atom(Listen,6,listen)
atom(Accept,6,accept)
atom(Bind,4,bind)
atom(Shutdown,8,shutdown)

#endif
