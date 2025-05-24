grammar labgramm;

options {
    language=C; 
    backtrack=true;
    memoize=true;
    output=AST;
    k=2;
    ASTLabelType=pANTLR3_BASE_TREE;
}

tokens{
	Source;
	FuncDef;
	FuncSignature;
	Init_List;
	ArgDef;
	Identifier;
	Array;
	Array_Size;
	Dimensions;
	Builtin_Type;
	Custom_Type;
	Body;
	VarDeclaration;
	Init_suffix;
	ArrayList;
	RepeatStatement;
	Expression;
	LoopStatement;
	ConditionStatement;
	BreakStatement;
	Condition;
	ReturnStatement;
	ElseStatement;
	Call;
	ListID;
	TypeRef;
	Braces;
	Indexer;
	AssignmentOP;
	Ident;
	Int;
	RETURN = 'return';
}


source  : sourceItem* -> ^(Source sourceItem*)
	;
sourceItem
  : funcDef;
 
funcDef  : funcSignature funcDefi -> ^(FuncDef funcSignature funcDefi)
	 ;

funcDefi :	blockStatement | ';'
	 ;

funcSignature 
	:	typeRef? identifier '(' init_declarator_list? ')' -> ^(FuncSignature typeRef? ^(Identifier identifier) init_declarator_list?)
	;
	
init_declarator_list
	:	argDef (',' argDef)* -> ^(Init_List argDef*)
	;
	
argDef	:	typeRef? identifier -> ^(ArgDef typeRef? ^(Identifier identifier))
	;

typeRef	: baseType arrayList* -> ^(TypeRef baseType arrayList*)
	;

baseType
	: builtin -> ^(Builtin_Type builtin)
	| custom 
	;
	
/*typeRefer	
	:	array | custom | builtin 
	;*/

custom 	:	identifier  -> ^(Custom_Type identifier)
	;

identifier
	:	 ID -> ^(ID);
	
/*arrayList
  :  (array (array)*) -> ^(ArrayList array)
  ;*/
arrayList
  :  (array (array)*) 
  ;

array 	:	 '[' size+=(',')* ']' -> ^(Array ^($size)*) 
;

//arrayl	: array -> ^(Array array);

statement 
	:	conditionStatement 
	|	assignmentStatement 
	|	blockStatement 
	|	whileStatement 
	|	doStatement 
	|	breakStatement 
	|	varStatement 
	|	returnStatement 
	|	expressionStatement 
	;

expressionStatement
	: expr ';'!
	;

breakStatement	: 'break' ';' -> ^(BreakStatement)
	;

doStatement	: 'do' blockStatement 'while' '(' expr ')' ';'  -> ^(RepeatStatement blockStatement ^(Expression expr))
	;

whileStatement	: 'while' '(' expr ')' statement  -> ^(LoopStatement ^(Expression expr) statement)
	;

blockStatement	: '{' statement* '}' -> ^(Body statement*)
		;

conditionStatement: 'if' '(' expr ')' statement (options {k=1; backtrack=false;}:elseStatement)? -> ^(ConditionStatement ^(Expression expr) statement elseStatement?)
		| 'switch' '(' expr ')' statement
	;

elseStatement	:	('else' statement) -> ^(ElseStatement statement)
	;
	
varStatement	: typeRef listid? ';' -> ^(VarDeclaration ^(Expression typeRef? listid?))
	;

returnStatement
  : RETURN expr ';' -> ^(ReturnStatement ^(Expression expr))
  ;
  
assignmentStatement
    : (identifier | indexer) (ASSIGNMENT_OPERATOR | Equal) expr ';' -> ^(AssignmentOP  ^(Expression ^(ASSIGNMENT_OPERATOR ^(Equal ^(Identifier identifier)? indexer expr))))
    ;

Equal
	:'='
	;

listid	:	(identifier | indexer) ('=' expr)? (',' (identifier | indexer) ('=' expr)?)* -> ^(ListID ^(Identifier identifier) indexer ^(Expression expr)? (^(Identifier identifier) indexer ^(Expression expr)?)*)
	;

place	: identifier -> ^(Identifier identifier)
	;

indexer	: identifier '[' indexer_list ']' -> ^(Indexer ^(Identifier identifier) ^(Init_List indexer_list)?)
	;

indexer_list
	: (expr((',' | ASSIGNMENT_OPERATOR) expr)*)?
	;


call	: identifier '(' call_list?')' -> ^(Call ^(Expression identifier ^(Init_List call_list)?))
	;

call_list
	: expr (','! expr)*
	;

braces	: '(' expr ')'
	;

expr  : assignmentExpression | braces | call | indexer ;

assignmentExpression
  : logicalOrExpr (ASSIGNMENT_OPERATOR^ assignmentExpression)?
  ;


logicalOrExpr
  : logicalAndExpr ('||'^ logicalAndExpr)*;

logicalAndExpr
  : inclusiveOrExpr ('&&'^ inclusiveOrExpr)*;
 
inclusiveOrExpr
  : exclusiveOrExpr ('|'^ exclusiveOrExpr)*;

exclusiveOrExpr
  : andExpr ('^'^ andExpr)*;

andExpr
  : equalityExpr ('&'^ equalityExpr)*;
  
equalityExpr
  : relationalExpr (('==' |'!=')^ relationalExpr)*; 
  
relationalExpr
  : shiftExpr (RELATIONAL_OPERATOR^ shiftExpr)*
  ;

shiftExpr
  : additiveExpr (('<<' | '>>')^ additiveExpr)*;
  
additiveExpr
  : multiplicativeExpr (('+' | '-')^ multiplicativeExpr)*;

multiplicativeExpr
  : unaryExpr (('*'|'/'|'%')^ unaryExpr)*;
  
unaryExpr
  : ('!'|'~')^ unaryExpr
  | (ADDITIVE_UNARY_OPERATOR^)? primaryExpr (ADDITIVE_UNARY_OPERATOR^)?;

primaryExpr
  : primitive initialization_suffix?
  | '('! expr ')'! 
  | '[' expr ']'
        |   '(' ')'
        |   '(' init_declarator_list ')'
        |   '.' ID
        |   '->' ID
  ;

initialization_suffix
  : identifier '(' exprList ')' -> ^(Init_suffix identifier Init_List)
  ;
  
exprList
  : expr (',' expr)*;

primitive
  : place | LITERAL | indexer;

builtin : BUILTINTYPE ;

LITERAL  
  : BOOL
  | BITS
  | HEX
  | DEC  // десятичный литерал
  | CHAR
  | STRING;

fragment BOOL   : ('true' | 'false');

fragment BITS  : '0' ('b' | 'B') ('0'..'1')+;

fragment HEX  : '0' ('x'|'X') (HEX_DIGIT)+;

fragment DEC   : ('0'..'9')+;

fragment CHAR:  '\'' ( ESC_SEQ | ~('\''|'\\') ) '\'' ;

BUILTINTYPE
  : 'bool'
  | 'byte'
  | 'int'
  | 'uint'
  | 'short'
  | 'long'
  | 'char'
  | 'string'
  | 'unsigned'
  | 'signed'
  | 'void'
  | 'float'
  | 'double';

ID  :	('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')* // идентификатор 
    ;

fragment
INT :	'0'..'9'+
    ;

ASSIGNMENT_OPERATOR
  : (('+'|'-'|'*'|'/'|'%'|'<<'|'>>'|'&'|'^'|'|')? '=');
  
    
RELATIONAL_OPERATOR
  : ('<'|'>'|'<='|'>=');

ADDITIVE_UNARY_OPERATOR
  : ('++'|'--'|'+'|'-');
  
fragment
STRING
    :  '"' ( ESC_SEQ | ~('\\'|'"') )* '"'
    ;

fragment
HEX_DIGIT : '0'..'9'|'a'..'f'|'A'..'F' ; // шестнадцатеричный литерал


//bits: "0[bB][01]+" ; // битовый литерал

fragment
ESC_SEQ
    :   '\\' ('b'|'t'|'n'|'f'|'r'|'\"'|'\''|'\\')
    |   UNICODE_ESC
    |   OCTAL_ESC
    ;

fragment
OCTAL_ESC
    :   '\\' ('0'..'3') ('0'..'7') ('0'..'7')
    |   '\\' ('0'..'7') ('0'..'7')
    |   '\\' ('0'..'7')
    ;


WS	:	( ' '
		| '\t'
		| '\r'
		| '\u000C'
		| '\n'
		) {$channel=HIDDEN;}
	;
	
COMMENT
    : '/*' ( options {greedy=false;} : . )* '*/' {$channel=HIDDEN;}
    ;
LINE_COMMENT
    : '//' ~('\n'|'\r')* '\r'? '\n' {$channel=HIDDEN;}
    ;
    
fragment
UNICODE_ESC
    :   '\\' 'u' HEX_DIGIT HEX_DIGIT HEX_DIGIT HEX_DIGIT
    ;