# SimpleRegex

## **Introduction**
A crued, simple regex library implemented in C++.

## **Supoorted Grammar**
|Meta charcter|Function|
|:----:|----|
|.|Matches any single character|
|*|Matches 0 or more repetitions of the preceding symbol.|
|+|Matches 1 or more repetitions of the preceding symbol.|
|?|Makes the preceding symbol optional.|
|\||Matches either the characters before or the characters after the symbol.|
|(R)|Character group. Matches regex R.|
|{n,m}|Matches at least `n` but not more than `m` repetitions of the preceding symbol.|

> 
> {n,n} matches for `n` times (`n` != `0`)
>
> {n,} matches at least `n`
>
> {,n} matches `0` to `n` times

|Meta charcter|Function|
|:----:|----|
|[ ]|Char set. Matches any character contained between the square brackets.|
|[^]|Char set. Matches any character that hasn't been contained between the square brackets.|
|\ |Escapes the next character.|

> You can use it to escapes character `[ ] ^ * + ? { } ( ) . \` and normal escapse character like `n t 0` in string.
>>
>> NOTICE: For normal escapse character, use `\ + character`. For example, use `"\n"` to represent newline.
>> 
>> For character defined in Grammar, use `\\ + character`. for example, use `"\\\\"` to represent `\`  and `"\\["` to represent`[`   

## Usage
*class Regex*
+ Generate regex

*class Regex_match*
+ Use class Regex to match string 

match type
+ match : Success when all character int the string match success
+ search : Success when the character from the begining of the string match success 
+ only support ASCLL

> ```Regex regex("a*"), regex_match("aab") => fail, regex_search("aab") => "aa"  ```

> ```Regex regex("a*"), regex_match("aaa") => "aaa", regex_search("aaab") => "aaa"  ``` (greedy)

## **Features that may added in the future**
+
|Shorthand|Description|
|:----:|----|
|.|Any character except new line|
|\w|Matches alphanumeric characters: `[a-zA-Z0-9_]`|
|\W|Matches non-alphanumeric characters: `[^\w]`|
|\d|Matches digits: `[0-9]`|
|\D|Matches non-digits: `[^\d]`|
|\s|Matches whitespace characters: `[\t\n\f\r\p{Z}]`|
|\S|Matches non-whitespace characters: `[^\s]`|

+
|Symbol|Description|
|:----:|----|
|?=|Positive Lookahead|
|?!|Negative Lookahead|
|?<=|Positive Lookbehind|
|?<!|Negative Lookbehind|
|^|Matches the beginning of the input.|
|$|Matches the end of the input.|

+ **Optional Greedy and Lazy Matching**

+ **Convert NFA to DFA**

> + *All of the features listed above are still a long way from being implemented.*
> + *But that's probably less than 20 years.*
