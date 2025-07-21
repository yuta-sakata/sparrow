#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/interpreter.h"

Value callFunction(Interpreter *interpreter, Function *function, Value *arguments, int argCount)
{
    if (function->arity != argCount)
    {
        runtimeError(interpreter, "期望 %d 个参数，但得到 %d 个。", function->arity, argCount);
        return createNull();
    }

    Environment env;
    initEnvironment(&env, function->closure);

    for (int i = 0; i < argCount; i++)
    {
        defineVariable(&env, function->paramNames[i], arguments[i]);
    }

    Environment *previous = interpreter->environment;
    interpreter->environment = &env;

    returnStatus.hasReturn = false;
    returnStatus.value = createNull();

    execute(interpreter, function->body);

    interpreter->environment = previous;
    freeEnvironment(&env);

    if (returnStatus.hasReturn)
    {
        Value result = returnStatus.value;
        returnStatus.hasReturn = false;
        return result;
    }

    return createNull();
}

Value evaluateCall(Interpreter *interpreter, Expr *expr)
{
    if (expr == NULL || expr->as.call.callee == NULL)
    {
        printf("ERROR: NULL callee in function call\n");
        return createNull();
    }

    Value callee = evaluate(interpreter, expr->as.call.callee);

    if (interpreter->hadError)
    {
        freeValue(callee);
        return createNull();
    }

    Value *arguments = NULL;
    if (expr->as.call.argCount > 0)
    {
        arguments = malloc(sizeof(Value) * expr->as.call.argCount);

        if (arguments == NULL)
        {
            freeValue(callee);
            runtimeError(interpreter, "内存分配失败");
            return createNull();
        }

        for (int i = 0; i < expr->as.call.argCount; i++)
        {
            if (expr->as.call.arguments[i] == NULL)
            {
                printf("ERROR: NULL argument %d in function call\n", i);
                for (int j = 0; j < i; j++)
                {
                    freeValue(arguments[j]);
                }
                free(arguments);
                freeValue(callee);
                return createNull();
            }

            arguments[i] = evaluate(interpreter, expr->as.call.arguments[i]);
        }
    }

    Value result;
    if (callee.type == VAL_FUNCTION)
    {
        if (callee.as.function == NULL)
        {
            printf("ERROR: NULL function pointer\n");
            result = createNull();
        }
        else
        {
            result = callFunction(interpreter, callee.as.function, arguments, expr->as.call.argCount);
        }
    }
    else if (callee.type == VAL_NATIVE_FUNCTION)
    {
        if (callee.as.nativeFunction == NULL)
        {
            printf("ERROR: NULL native function pointer\n");
            result = createNull();
        }
        else
        {
            if (callee.as.nativeFunction->function != NULL)
            {
                result = callee.as.nativeFunction->function(expr->as.call.argCount, arguments);
            }
            else
            {
                printf("ERROR: Native function has NULL function pointer\n");
                result = createNull();
            }
        }
    }
    else
    {
        runtimeError(interpreter, "只能调用函数。");
        result = createNull();
    }

    // 问题在这里：需要检查 arguments 是否为 NULL
    if (arguments != NULL)
    {
        for (int i = 0; i < expr->as.call.argCount; i++)
        {
            freeValue(arguments[i]);
        }
        free(arguments);
    }

    // 还需要释放 callee
    freeValue(callee);

    return result;
}