#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

/**
 * \file
 * \brief
 * \author Lectem
 */

namespace YAECS {
	class Space;
	class System
	{
		friend class Space;
		virtual void update(Space &) = 0;
	public:
		virtual ~System() {}
	};
}

#endif // SYSTEM_H_INCLUDED
