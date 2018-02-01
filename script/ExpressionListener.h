class ScriptExpression;

class ExpressionListener
{
public:
	virtual ~ExpressionListener() {}
	virtual void expressionValueChanged(ScriptExpression *) {}
	virtual void expressionStateChanged(ScriptExpression *) {}
};