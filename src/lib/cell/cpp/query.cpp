#include <set>
#include <stdexcept>

#include <common/cpp/expected.h>
#include <cell/cpp/query.h>

namespace lattice
{
namespace cell
{

typedef std::stack<actions::node*> node_stack_type;

/**
 * Gets the data value from a node.
 *
 * Currently assumes that the node is a literal.
 *
 * @param node: The node to get the value from.
 */
expected<data_value> get_value(actions::node* node)
{
	if (node->get_type() != actions::node::node_type::LITERAL)
		{
			return expected<data_value>::from_exception(
					std::invalid_argument("node is not a literal value."));
		}

	auto* l = dynamic_cast<actions::literal*>(node);

	if (l == nullptr)
		{
			return expected<data_value>::from_exception(
					std::invalid_argument(
							"node claims to be a literal value, but underlying type is not."));
		}

	return expected<data_value>(l->get_value());

}

/**
 * Pushes a new literal value onto the stack.
 *
 * @param value: The value to wrap a literal around.
 * @param s: The stack to push it on.
 */
void push_value(expected<data_value> value, node_stack_type& s)
{
	if (!value.valid())
		{
			// Deal with this
			return;
		}

	// Leak!
	s.push(new actions::literal(value.get()));
}

void evaluate(actions::node::node_type type, node_stack_type& s)
{

	auto l = get_value(s.top());
	s.pop();
	auto r = get_value(s.top());
	s.pop();

	// Make sure that both arguments are valid.
	if (!l.valid() || !r.valid())
		{
			return;
		}

	switch (type)
		{
	case actions::node::node_type::OP_ADD:
		push_value(l.get() + r.get(), s);
		break;
	case actions::node::node_type::OP_SUB:
		push_value(l.get() - r.get(), s);
		break;
	case actions::node::node_type::OP_MUL:
		push_value(l.get() * r.get(), s);
		break;
	case actions::node::node_type::OP_DIV:
		push_value(l.get() / r.get(), s);
		break;
	default:
		throw std::invalid_argument("unknown operation requested in select expression evaluator.");
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

	// Set the current node.
	actions::node* cur_node = sg.se;

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

						cur_node=s.top();
						s.pop();
					}
				break;

				// This is a leaf node. Pop the top of the stack,
				// and replace it with the the current node.
			case actions::node::node_type::LITERAL:
			case actions::node::node_type::COLUMN_REF:
			case actions::node::node_type::TABLE_REF:
				{
					if (s.empty())
						{
							auto results = get_value(cur_node);
							return results.get();
						}

					visited.insert(cur_node);

					auto *tmp = cur_node;
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
			auto result = dispatch(se);
			tpl.push_back(result.to_string());
		}

	return tpl;
}

} // namespace cell
} // namespace lattice

