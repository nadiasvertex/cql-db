#include <set>
#include <stdexcept>

#include <cell/cpp/query.h>

namespace lattice
{
namespace cell
{

typedef std::stack<actions::node*> node_stack_type;

void evaluate(actions::node::node_type type, node_stack_type& s)
{
switch (type)
	{
case actions::node::node_type::OP_ADD:

	break;
case actions::node::node_type::OP_SUB:
	break;
case actions::node::node_type::OP_MUL:
	break;
case actions::node::node_type::OP_DIV:
	break;
	}
}

data_value query::dispatch(select_group_type& sg)
{
// The evaluation stack. We evaluate
// iteratively instead of recursively to
// save space and time.
node_stack_type s;

// Store nodes that have been visited.
std::set<actions::node*> visited;

// Start the stack
s.push(sg.se);

actions::node* cur_node = s.top();

while (true)
	{
		switch (cur_node->get_type())
			{
		case actions::node::node_type::OP_ADD:
		case actions::node::node_type::OP_SUB:
		case actions::node::node_type::OP_MUL:
		case actions::node::node_type::OP_DIV:
			if (visited.find(cur_node) == visited.end())
				{
					actions::binop* op = dynamic_cast<actions::binop*>(cur_node);

					// Op is apparently not really a binop.
					if (op == nullptr)
						{
							throw std::invalid_argument(
									"Evaluating a node that claims to be a binop, but dynamic cast yields nullptr.");
						}

					// If the left side hasn't been visited, try it first.
					if (visited.find(op->get_left()) == visited.end())
						{
							s.push(cur_node);
							cur_node = op->get_left();
							continue;
						}

					// If the right side hasn't been visited, try it next.
					if (visited.find(op->get_right()) == visited.end())
						{
							s.push(cur_node);
							cur_node = op->get_right();
							continue;
						}

					visited.insert(cur_node);
					evaluate(cur_node->get_type(), s);
				}
			break;

			// This is a leaf node. Pop the top of the stack,
			// and replace it with the the current node.
		case actions::node::node_type::LITERAL:
		case actions::node::node_type::COLUMN_REF:
			{
				auto *tmp = cur_node;
				visited.insert(cur_node);
				cur_node = s.top();
				s.pop();
				s.push(tmp);
			}
			break;
			}
	}
}

query::tuple_type query::solve_once()
{
tuple_type tpl;

// 1. Execute predicates

// 2. Solve selects

for (auto& se : select_exprs)
	{

	}
}

} // namespace cell
} // namespace lattice

