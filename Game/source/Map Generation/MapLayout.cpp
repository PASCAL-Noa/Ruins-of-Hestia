#include "Map Generation/MapLayout.h"

#include "Map Generation/MapMask.h"


namespace GPC {
    void MapLayout::Link(glm::ivec2 a, glm::ivec2 b) {
        glm::ivec2 diff = b - a; // a => b
		if (diff.x == 0 && diff.y == 0) return;
        if (diff.x == -1) {
            m_Layout[a.x][a.y] |= MASK_LEFT;
            m_Layout[b.x][b.y] |= MASK_RIGHT;
        }
        else if (diff.x == 1) {
            m_Layout[a.x][a.y] |= MASK_RIGHT;
            m_Layout[b.x][b.y] |= MASK_LEFT;
        }
        else if (diff.y == -1) {
            m_Layout[a.x][a.y] |= MASK_UP;
            m_Layout[b.x][b.y] |= MASK_DOWN;
        }
        else if (diff.y == 1) {
            m_Layout[a.x][a.y] |= MASK_DOWN;
            m_Layout[b.x][b.y] |= MASK_UP;
        }
    }

    void MapLayout::GetMoveExcluded(glm::ivec2 position, const set_ivec2 &excluded, const MapMask &map_mask, glm::ivec2 &out_direction) {
    	static RoomMask masks[DIRECTION_INDEX_COUNT + 1] = { MASK_UP, MASK_DOWN, MASK_RIGHT, MASK_LEFT, 0 };
    	static glm::ivec2 direction[DIRECTION_INDEX_COUNT + 1] = { { 0, -1 }, { 0, 1 }, { 1, 0 }, { -1, 0 }, { 0, 0 } };

    	uint8_t validDirectionCount = 0;
    	DirectionIndex valid[4]{ DIRECTION_INDEX_COUNT, DIRECTION_INDEX_COUNT, DIRECTION_INDEX_COUNT, DIRECTION_INDEX_COUNT };

    	for (uint8_t i = 0; i < DIRECTION_INDEX_COUNT; ++i)
    	{
    		glm::ivec2 post_move = position + direction[i];
    		if (post_move.x < 0 || post_move.y < 0 || post_move.x > MAX_MAP_SIZE - 1 || post_move.y > MAX_MAP_SIZE - 1) continue;
    		if ((excluded.contains(post_move) == false) && map_mask.Get(post_move.x, post_move.y))
    		{
    			valid[validDirectionCount++] = i;
    		}
    	}

    	DirectionIndex index = (validDirectionCount == 0 ? DIRECTION_INDEX_COUNT : valid[rand() % validDirectionCount]);
    	out_direction = direction[index];
    }

    void MapLayout::GetMoveIncluded(glm::ivec2 position, const set_ivec2 &included, const MapMask &map_mask, glm::ivec2 &out_direction) {
    	static RoomMask masks[DIRECTION_INDEX_COUNT + 1] = { MASK_UP, MASK_DOWN, MASK_RIGHT, MASK_LEFT, 0 };
    	static glm::ivec2 direction[DIRECTION_INDEX_COUNT + 1] = { { 0, -1 }, { 0, 1 }, { 1, 0 }, { -1, 0 }, { 0, 0 } };

    	uint8_t validDirectionCount = 0;
    	DirectionIndex valid[4]{ DIRECTION_INDEX_COUNT, DIRECTION_INDEX_COUNT, DIRECTION_INDEX_COUNT, DIRECTION_INDEX_COUNT };

    	for (uint8_t i = 0; i < DIRECTION_INDEX_COUNT; ++i)
    	{
    		glm::ivec2 post_move = position + direction[i];
    		if (post_move.x < 0 || post_move.y < 0 || post_move.x > MAX_MAP_SIZE - 1 || post_move.y > MAX_MAP_SIZE - 1) continue;
    		if (included.contains(post_move) && map_mask.Get(post_move.x, post_move.y))
    		{
    			valid[validDirectionCount++] = i;
    		}
    	}

    	DirectionIndex index = (validDirectionCount == 0 ? DIRECTION_INDEX_COUNT : valid[rand() % validDirectionCount]);
    	out_direction = direction[index];
    }

    int MapLayout::TryResolve(const MapMask &map_mask) {
    	// Start at the top-left most
		glm::ivec2 start = { MAX_MAP_SIZE - 1, MAX_MAP_SIZE - 1 };
		// End at the bottom-right most
		glm::ivec2 end = { 0, 0 };
		for (int32_t y = 0; y < MAX_MAP_SIZE; ++y)
		{
			for (int32_t x = 0; x < MAX_MAP_SIZE; ++x)
			{
				if (map_mask.Get(x, y) == false) continue;

				if (x < start.x && y < start.y) start = { x, y };
				if (x > end.x && y > end.y) end = { x, y };
			}
		}
		m_Start = start;
    	m_End = end;

		set_ivec2 start_path{};
		set_ivec2 end_path{};

		glm::ivec2 current_pointer_start = m_Start;
		glm::ivec2 current_pointer_end = m_End;
		start_path.insert(current_pointer_start);
		end_path.insert(current_pointer_end);

		// Linking D - A
		while (
			start_path.contains(current_pointer_end) == false && 
			end_path.contains(current_pointer_start) == false
		) {
			glm::ivec2 start_path_direction;
			GetMoveExcluded(current_pointer_start, start_path, map_mask, start_path_direction);
			glm::ivec2 new_pointer_start = current_pointer_start + start_path_direction;
			Link(current_pointer_start, new_pointer_start);
			start_path.insert(new_pointer_start);
			current_pointer_start = new_pointer_start;

			glm::ivec2 end_path_direction;
			GetMoveExcluded(current_pointer_end, end_path, map_mask, end_path_direction);
			glm::ivec2 new_pointer_end = current_pointer_end + end_path_direction;
			Link(current_pointer_end, new_pointer_end);
			end_path.insert(new_pointer_end);
			current_pointer_end = new_pointer_end;

			// PrintLayout(*this);

			if (start_path_direction.x == 0 && start_path_direction.y == 0 && end_path_direction.x == 0 && end_path_direction.y == 0) return 0;
		}

		set_ivec2 acquired{};
		std::ranges::set_union(start_path, end_path, std::inserter(acquired, acquired.begin()), set_ivec2_cmp);

		std::vector<glm::ivec2> remaining{};
		for (int32_t y = 0; y < MAX_MAP_SIZE; ++y)
		{
			for (int32_t x = 0; x < MAX_MAP_SIZE; ++x)
			{
				glm::ivec2 current{ x, y };
				if (Get(x, y) == MASK_EMPTY && map_mask.Get(current.x,current.y))
				{
					remaining.push_back(current);
				}
			}
		}
		auto iteration_count = remaining.size();

    	uint32_t count_failed = 0;
		while (iteration_count > 0) {
			uint32_t it_index = rand() % iteration_count;
			glm::ivec2 current = remaining[it_index];

			glm::ivec2 direction;
			GetMoveIncluded(current, acquired, map_mask, direction);

			if (direction.x == 0 && direction.y == 0) {
				count_failed++;
				if (count_failed >= iteration_count) {
					return 0;
				}
				continue;
			}
			count_failed = 0;

			remaining[it_index] = remaining[iteration_count - 1];
			glm::ivec2 next = current + direction;
			Link(current, next);
			acquired.insert(current);

			// PrintLayout(*this);
			iteration_count--;
		}

		return 1;
    }

    void MapLayout::Resolve(const MapMask &map_mask) {
    	uint32_t attempt_failed = 0;
    	while (TryResolve(map_mask) == false) {attempt_failed++;};
    	GPC_INFO << "Resolve Attempt Failed Count : " << attempt_failed << ENDL;
    }

    RoomMask MapLayout::Get(int32_t x, int32_t y) const {
    	if (x >= 0 || y >= 0 || x < MAX_MAP_SIZE || y < MAX_MAP_SIZE) return m_Layout[x][y];
    	return MASK_EMPTY;
    }

    const MaskLine & MapLayout::operator[](uint32_t x) const {
        return m_Layout[x];
    }

    void PrintLayout(const MapLayout &layout) {

        for (uint32_t x = 0; x < MAX_MAP_SIZE * 3 + 2; ++x)
        {
            std::cout << "-";
        }
        std::cout << std::endl;
        for (uint32_t y = 0; y < MAX_MAP_SIZE; ++y)
        {
            std::cout << "|";
            for (uint32_t x = 0; x < MAX_MAP_SIZE; ++x)
            {
                std::cout << " ";
                if (layout[x][y] & MASK_UP) {
                    std::cout << "|";
                }
                else {
                    std::cout << " ";
                }
                std::cout << " ";
            }
            std::cout << "|";
            std::cout << std::endl;
            std::cout << "|";
            for (uint32_t x = 0; x < MAX_MAP_SIZE; ++x)
            {
                if (layout[x][y] & MASK_LEFT) {
                    std::cout << "-";
                }
                else {
                    std::cout << " ";
                }
            	if (x == layout.GetStart().x && y == layout.GetStart().y) {
            		std::cout << "S";
            	} else if (x == layout.GetEnd().x && y == layout.GetEnd().y) {
            		std::cout << "E";
            	} else {
            		std::cout << "+";
            	}
                if (layout[x][y] & MASK_RIGHT) {
                    std::cout << "-";
                }
                else {
                    std::cout << " ";
                }
            }
            std::cout << "|";
            std::cout << std::endl;
            std::cout << "|";
            for (uint32_t x = 0; x < MAX_MAP_SIZE; ++x)
            {
                std::cout << " ";
                if (layout[x][y] & MASK_DOWN) {
                    std::cout << "|";
                }
                else {
                    std::cout << " ";
                }
                std::cout << " ";
            }
            std::cout << "|";
            std::cout << std::endl;
        }
        for (uint32_t x = 0; x < MAX_MAP_SIZE * 3 + 2; ++x)
        {
            std::cout << "-";
        }
        std::cout << std::endl;
    }
} // GPC