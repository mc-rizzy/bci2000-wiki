////////////////////////////////////////////////////////////////////////////////
// $Id: InterpreterExpression.cpp 9216 2026-01-01 14:20:40Z mellinger $
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: An expression that is linked to a command interpreter object.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2026: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "InterpreterExpression.h"

#include "BCIException.h"
#include "CommandInterpreter.h"
#include "StateMachine.h"

InterpreterExpression::InterpreterExpression(CommandInterpreter &inInterpreter, const std::string &inExpr)
    : Expression(inExpr.empty() ? inInterpreter.GetRemainingTokens() : inExpr), mrInterpreter(inInterpreter),
      mAllowAssignment(true), mRefersStates(false), mSample(0)
{
    ThrowOnError(true);
    Compile(inInterpreter.ExpressionVariables());
}

double InterpreterExpression::Execute(int inSample) const
{
    mSample = inSample;
    return Expression::Execute(&mrInterpreter.StateMachine().ControlSignal(), mSample);
}

bool InterpreterExpression::StateExists(CommandInterpreter &inInterpreter, const std::string &inName)
{
    return inInterpreter.StateMachine().States().Exists(inName);
}

void InterpreterExpression::AssertState(CommandInterpreter &inInterpreter, const std::string &inName)
{
    if (!StateExists(inInterpreter, inName))
        throw bciexception << "State \"" << inName << "\" does not exist";
}

void InterpreterExpression::AssertAssignment() const
{
    if (!mAllowAssignment)
        throw bciexception << "Assignment not supported in current context";
}

Expression::Node *InterpreterExpression::Variable(const std::string &inName)
{
    if (StateExists(mrInterpreter, inName))
        return State(inName);
    return ArithmeticExpression::Variable(inName);
}

Expression::Node *InterpreterExpression::VariableAssignment(const std::string &inName, Node *inRhs)
{
    AssertAssignment();
    if (StateExists(mrInterpreter, inName))
        return StateAssignment(inName, inRhs);
    return ArithmeticExpression::VariableAssignment(inName, inRhs);
}

Expression::Node *InterpreterExpression::State(const std::string &inName)
{
    mRefersStates = true;
    return new StateNode(mrInterpreter, inName, mSample);
}

Expression::Node *InterpreterExpression::StateAssignment(const std::string &inName, Node *inRhs)
{
    AssertAssignment();
    mRefersStates = true;
    return new StateAssignmentNode(mrInterpreter, inName, inRhs);
}

// Nodes
InterpreterExpression::StateNode::StateNode(CommandInterpreter& interpreter, const std::string& name, const int& sample)
    : mrInterpreter(interpreter), mName(name), mrSample(sample), mpState(nullptr),
    mOnStateDeleted(&StateNode::OnStateDeleted, this)
{
}

InterpreterExpression::StateNode::~StateNode()
{
    if (mpState)
        mpState->RemoveListener(State::OnDelete, &mOnStateDeleted);
}

double InterpreterExpression::StateNode::OnEvaluate()
{
    std::unique_lock<StateMachine> lock(mrInterpreter.StateMachine());
    if (!mpState) {
        AssertState(mrInterpreter, mName);
        if (!mrInterpreter.StateMachine().States().ValidPositions())
            return 0;
        mpState = &mrInterpreter.StateMachine().States().ByName(mName);
        mpState->AddListener(State::OnDelete, &mOnStateDeleted);
    }
    return mrInterpreter.StateMachine().StateVector().StateValue(mpState->Location(), mpState->Length(), mrSample);
}

void InterpreterExpression::StateNode::OnStateDeleted()
{
    mpState = nullptr;
}

InterpreterExpression::StateAssignmentNode::StateAssignmentNode(CommandInterpreter& interpreter, const std::string& name, Node* rhs)
    : mrInterpreter(interpreter), mName(name)
{
    Add(rhs);
}

InterpreterExpression::StateAssignmentNode::~StateAssignmentNode()
{
}

double InterpreterExpression::StateAssignmentNode::OnEvaluate()
{
    double rhs = mChildren[0]->Evaluate();
    std::unique_lock<StateMachine> lock(mrInterpreter.StateMachine());
    AssertState(mrInterpreter, mName);
    if (!mrInterpreter.StateMachine().SetStateValue(mName, static_cast<State::ValueType>(rhs)))
        throw bciexception << "Could not set value of state " << mName;
    return rhs;
}
