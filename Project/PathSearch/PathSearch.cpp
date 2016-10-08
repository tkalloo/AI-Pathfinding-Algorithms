#include "PathSearch.h"
#include <chrono> 

namespace fullsail_ai { namespace algorithms {

	PathSearch::PathSearch()
	{
		NumRows = 0;
		NumCols = 0;
		SolutionFound = false; 
		FirstTile = nullptr; 
		LastTile = nullptr; 
		startingNode = nullptr;
		goalNode = nullptr;
	}

	PathSearch::~PathSearch()
	{
		//TODO final cleanup 

		auto iter = SearchMap.begin();
		for (iter; iter != SearchMap.end(); iter++)
		{
			for (auto edgeIter = (*iter).second->edges.begin(); edgeIter != (*iter).second->edges.end(); edgeIter++)
			{
				Edge * tempEdge = (*edgeIter);
				delete tempEdge;
			}
			SearchNode * temp = (*iter).second;
			delete temp;
		}
		SearchMap.clear();

		auto VisitedIter = Visited.begin();
		for (VisitedIter; VisitedIter != Visited.end(); VisitedIter++)
		{
			PlannerNode* tempPlanner = (*VisitedIter).second;
			delete tempPlanner;
		}
		Visited.clear(); //Deleted in visited means deleted in open
		open.clear();
	}

	float GetDistance(Tile const* start, Tile const* end)
	{
		return  sqrtf(( (end->getXCoordinate() - start->getXCoordinate()) * (end->getXCoordinate() - start->getXCoordinate()))
			+ ((end->getYCoordinate() - start->getYCoordinate()) * (end->getYCoordinate() - start->getYCoordinate())));
	}

	void PathSearch::initialize(TileMap* _tileMap)
	{
		if (_tileMap)
		{
			NumRows = _tileMap->getRowCount();
			NumCols = _tileMap->getColumnCount(); 
			//loop through tilemap, create a search node for each tile
			for (int row = 0; row < NumRows; row++)
			{
				for (int col = 0; col <NumCols; col++)
				{
					if (nullptr != _tileMap->getTile(row, col) && nullptr == SearchMap[_tileMap->getTile(row, col)])
					{	
						SearchMap[_tileMap->getTile(row, col)] = new SearchNode(_tileMap->getTile(row, col));  // newNode;
					}
				}
			}

			//Get Adjacent Tiles
			int row = 0; 
			int col = 0;
			for (int row = 0; row < NumRows; row++)
			{
				for (int col = 0; col < NumCols; col++)
				{
					if (SearchMap[_tileMap->getTile(row, col)]->tile->getRow() % 2 == 0) //Even numbered tile
					{
						if (_tileMap->getTile(row - 1, col - 1))
							SearchMap[_tileMap->getTile(row, col)]->edges.push_back(new Edge(SearchMap[_tileMap->getTile(row - 1, col - 1)], (float)_tileMap->getTile(row - 1, col - 1)->getWeight() * GetDistance(_tileMap->getTile(row, col), _tileMap->getTile(row - 1, col - 1))));

						if (_tileMap->getTile(row - 1, col))
							SearchMap[_tileMap->getTile(row, col)]->edges.push_back(new Edge(SearchMap[_tileMap->getTile(row - 1, col)], GetDistance(_tileMap->getTile(row, col), _tileMap->getTile(row - 1, col))*(float)_tileMap->getTile(row - 1, col)->getWeight()));

						if (_tileMap->getTile(row, col + 1))
							SearchMap[_tileMap->getTile(row, col)]->edges.push_back(new Edge(SearchMap[_tileMap->getTile(row, col + 1)], GetDistance(_tileMap->getTile(row, col), _tileMap->getTile(row, col + 1))*(float)_tileMap->getTile(row, col + 1)->getWeight()));

						if (_tileMap->getTile(row + 1, col))
							SearchMap[_tileMap->getTile(row, col)]->edges.push_back(new Edge(SearchMap[_tileMap->getTile(row + 1, col)], GetDistance(_tileMap->getTile(row, col), _tileMap->getTile(row + 1, col))*(float)_tileMap->getTile(row + 1, col)->getWeight() ));

						if (_tileMap->getTile(row + 1, col - 1))
							SearchMap[_tileMap->getTile(row, col)]->edges.push_back(new Edge(SearchMap[_tileMap->getTile(row + 1, col - 1)], GetDistance(_tileMap->getTile(row, col), _tileMap->getTile(row + 1, col - 1)) * (float)_tileMap->getTile(row + 1, col - 1)->getWeight()));

						if ( _tileMap->getTile(row, col - 1))
							SearchMap[_tileMap->getTile(row, col)]->edges.push_back(new Edge(SearchMap[_tileMap->getTile(row, col - 1)], GetDistance(_tileMap->getTile(row, col), _tileMap->getTile(row, col - 1)) * (float)_tileMap->getTile(row, col - 1)->getWeight()));
					}
					else if (SearchMap[_tileMap->getTile(row, col)]->tile->getRow() % 2 == 1) //Odd numbered tile
					{
						if (_tileMap->getTile(row - 1, col))
							SearchMap[_tileMap->getTile(row, col)]->edges.push_back(new Edge(SearchMap[_tileMap->getTile(row - 1, col)], GetDistance(_tileMap->getTile(row, col), _tileMap->getTile(row - 1, col))* (float)_tileMap->getTile(row - 1, col)->getWeight()));

						if (_tileMap->getTile(row - 1, col + 1))
							SearchMap[_tileMap->getTile(row, col)]->edges.push_back(new Edge(SearchMap[_tileMap->getTile(row - 1, col + 1)], GetDistance(_tileMap->getTile(row, col), _tileMap->getTile(row - 1, col + 1))* (float)_tileMap->getTile(row - 1, col + 1)->getWeight()));

						if (_tileMap->getTile(row, col - 1))
							SearchMap[_tileMap->getTile(row, col)]->edges.push_back(new Edge(SearchMap[_tileMap->getTile(row, col - 1)], GetDistance(_tileMap->getTile(row, col), _tileMap->getTile(row, col - 1))*(float)_tileMap->getTile(row, col - 1)->getWeight()));

						if (_tileMap->getTile(row, col + 1))
							SearchMap[_tileMap->getTile(row, col)]->edges.push_back(new Edge(SearchMap[_tileMap->getTile(row, col + 1)], GetDistance(_tileMap->getTile(row, col), _tileMap->getTile(row, col + 1))* (float)_tileMap->getTile(row, col + 1)->getWeight()));

						if (_tileMap->getTile(row + 1, col))
							SearchMap[_tileMap->getTile(row, col)]->edges.push_back(new Edge(SearchMap[_tileMap->getTile(row + 1, col)], GetDistance(_tileMap->getTile(row, col), _tileMap->getTile(row + 1, col))* (float)_tileMap->getTile(row + 1, col)->getWeight()  ));

						if (_tileMap->getTile(row + 1, col + 1))
							SearchMap[_tileMap->getTile(row, col)]->edges.push_back(new Edge(SearchMap[_tileMap->getTile(row + 1, col + 1)], GetDistance(_tileMap->getTile(row, col), _tileMap->getTile(row + 1, col + 1))*(float)_tileMap->getTile(row + 1, col + 1)->getWeight()));

					}
					//col++; 
				} //while (col < NumCols);
				//row++; 
			} //while (row < NumRows);
		}
		//where search graph is generated
	}

	void PathSearch::enter(int startRow, int startColumn, int goalRow, int goalColumn)
	{
		if (nullptr == FirstTile && nullptr == LastTile)
		{
			std::unordered_map<Tile*, SearchNode*>::iterator iter = SearchMap.begin();
			do
			{
				if (iter->first->getRow() == startRow && iter->first->getColumn() == startColumn)
				{
					iter->second->isFirst = true;
					FirstTile = iter->second->tile;
				}
				else if (iter->first->getRow() == goalRow && iter->first->getColumn() == goalColumn)
				{
					iter->second->isLast = true;
					LastTile = iter->second->tile;
				}
				iter++; 
			} while (iter != SearchMap.end());

		}
	}


	void PathSearch::update(long timeslice)
	{
		auto startTimer = std::chrono::high_resolution_clock::now();
		
		//Find starting Node 
		if (nullptr == startingNode && nullptr == goalNode)
		{
			std::unordered_map<Tile*, SearchNode*>::iterator iter = SearchMap.begin();

			do
			{
				if (iter->second->isFirst)
					startingNode = iter->second;

				if (iter->second->isLast)
					goalNode = iter->second;
				iter++;
			} while (iter != SearchMap.end());

			open.push(new PlannerNode(startingNode)); //problem is here, keeps resetting at beginning
			Visited[startingNode] = open.front(); //Current state is the goal 
			open.front()->givenCost = 0.0f; 
			Visited[startingNode]->heuristicCost = GetDistance(startingNode->tile, goalNode->tile); 
			open.front()->finalCost = Visited[startingNode]->heuristicCost * 1.2f; //Heuristic weight
		}
		do
		{
			PlannerNode * current = open.front(); //crashes here when clicking double arrow twice
			open.pop();
			if (current->vertex->tile->getWeight() == 0)
				continue;

			if (LastTile == current->vertex->tile) ///Build the solution list from the tiles and exit
			{
				SolutionVector.push_back(LastTile);

				Tile* tempForLine = current->vertex->tile; 
				PlannerNode* tempForDrawing = current->parent;
				do
				{
					SolutionVector.push_back(tempForDrawing->vertex->tile); 
					//tempForDrawing->vertex->tile->addLineTo(tempForLine, 0x00000000); 
					tempForLine = tempForDrawing->vertex->tile;
					tempForDrawing = tempForDrawing->parent; 
				} while (tempForDrawing->vertex->tile != FirstTile);

				SolutionVector.push_back(FirstTile);
				SolutionFound = true;
				break;
			}
			auto iter = current->vertex->edges.begin();
			do //for (iter; iter != current->vertex->edges.end(); iter++)//while (false == current->vertex->edges.empty()) unsigned int i = 0; i < current->vertex->edges.size(); i++
			{
				SearchNode * successor = (*iter)->EndPoint; //Only go through neighboring tiles
				float tempGivenCost = current->givenCost + (*iter)->cost; 
				if (Visited[successor]/* && successor*/)
				{
					if (tempGivenCost < Visited[successor]->givenCost)
					{
						PlannerNode * successorNode = Visited[successor];
						open.remove(successorNode);
						successorNode->givenCost = tempGivenCost; 
						successorNode->finalCost = tempGivenCost + successorNode->heuristicCost * 1.2f;
						successorNode->parent = current; 
						open.push(successorNode); 
					}
				}
				else if (successor && successor->tile->getWeight() != 0)
				{
					PlannerNode * successorNode = new PlannerNode(successor);
					successorNode->parent = current; 
					successorNode->heuristicCost = GetDistance(successorNode->vertex->tile, goalNode->tile);
					successorNode->givenCost = tempGivenCost; 
					successorNode->finalCost = tempGivenCost + successorNode->heuristicCost * 1.2f;

					Visited[successor] = successorNode;
					//successorNode->vertex->tile->setFill(0xFFFF0000);
					open.push(successorNode); 
				}
				iter++; 
			} while (iter != current->vertex->edges.end());
			if (0 == timeslice || timeslice > 5000)
				break;
			auto endTimer = std::chrono::high_resolution_clock::now();
			auto timeDuration = (endTimer - startTimer); 
			std::chrono::duration_cast<std::chrono::milliseconds>(timeDuration);
			if ((timeDuration.count() /1000) > timeslice)
				break; 

		}while (false == open.empty());
	}

	void PathSearch::exit()
	{
		FirstTile = nullptr;
		LastTile = nullptr;
		startingNode = nullptr;
		goalNode = nullptr;
		auto VisitedIter = Visited.begin();
		for (VisitedIter; VisitedIter != Visited.end(); VisitedIter++)
		{
			PlannerNode* tempPlanner = (*VisitedIter).second;
			delete tempPlanner;
		}
		Visited.clear(); //Deleted in visited means deleted in open
		SolutionVector.clear(); 
		open.clear(); 
	}

	void PathSearch::shutdown()
	{
		//Clean up searchnode memory 
		SolutionFound = false;
		FirstTile = nullptr;
		LastTile = nullptr;
		startingNode = nullptr;
		goalNode = nullptr;
		auto iter = SearchMap.begin();
		for (iter; iter != SearchMap.end(); iter++)
		{
			for (auto edgeIter = (*iter).second->edges.begin(); edgeIter != (*iter).second->edges.end(); edgeIter++)
			{
				Edge * tempEdge = (*edgeIter);
				delete tempEdge;
			}
			SearchNode * temp = (*iter).second;
			delete temp;
		}
		SearchMap.clear();
	}

	bool PathSearch::isDone() const
	{
		return SolutionFound;
	}

	std::vector<Tile const*> const PathSearch::getSolution() const
	{
		return SolutionVector; 
	}
}}  // namespace fullsail_ai::algorithms

