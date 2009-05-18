/*
    This file is part of Cute Chess.

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "chessboard.h"

using namespace Chess;


void Board::generateMoves(QVector<Chess::Move>& moves) const
{
	// Cut the wall squares (the ones with a value of InvalidPiece) off
	// from the squares to iterate over. It bumps the speed up a bit.
	unsigned begin = m_arwidth * 2;
	unsigned end = m_squares.size() - begin;
	
	moves.clear();
	
	for (unsigned sq = begin; sq < end; sq++) {
		Piece piece = m_squares[sq];
		if (piece.side() != m_side)
			continue;
		
		switch (piece.type()) {
		case Piece::Pawn:
			generatePawnMoves(sq, moves);
			break;
		case Piece::Knight:
			generateHoppingMoves(sq, m_knightOffsets, moves);
			break;
		case Piece::Bishop:
			generateSlidingMoves(sq, m_bishopOffsets, moves);
			break;
		case Piece::Rook:
			generateSlidingMoves(sq, m_rookOffsets, moves);
			break;
		case Piece::Queen:
			generateSlidingMoves(sq, m_bishopOffsets, moves);
			generateSlidingMoves(sq, m_rookOffsets, moves);
			break;
		case Piece::Archbishop:
			generateSlidingMoves(sq, m_bishopOffsets, moves);
			generateHoppingMoves(sq, m_knightOffsets, moves);
			break;
		case Piece::Chancellor:
			generateSlidingMoves(sq, m_rookOffsets, moves);
			generateHoppingMoves(sq, m_knightOffsets, moves);
			break;
		case Piece::King:
			generateHoppingMoves(sq, m_bishopOffsets, moves);
			generateHoppingMoves(sq, m_rookOffsets, moves);
			generateCastlingMoves(moves);
			break;
		default:
			qFatal("Board::generateMoves(): invalid piece type");
		}
	}
}

QVector<Move> Board::legalMoves()
{
	QVector<Move> moves;
	
	// Generate pseudo-legal moves
	generateMoves(moves);
	// Erase all illegal moves from the vector
	for (int i = moves.size() - 1; i >= 0; i--) {
		makeMove(moves[i]);
		if (!isLegalPosition())
			moves.erase(moves.begin() + i);
		undoMove();
	}
	
	return moves;
}

void Board::addPromotions(int sourceSquare,
                          int targetSquare,
                          QVector<Chess::Move>& moves) const
{
	moves.push_back(Move(sourceSquare, targetSquare, Piece::Knight));
	moves.push_back(Move(sourceSquare, targetSquare, Piece::Bishop));
	moves.push_back(Move(sourceSquare, targetSquare, Piece::Rook));
	moves.push_back(Move(sourceSquare, targetSquare, Piece::Queen));
	
	if (m_variant.isCapablanca()) {
		moves.push_back(Move(sourceSquare, targetSquare, Piece::Archbishop));
		moves.push_back(Move(sourceSquare, targetSquare, Piece::Chancellor));
	}
}

void Board::generatePawnMoves(int sourceSquare,
                              QVector<Chess::Move>& moves) const
{
	int targetSquare;
	Piece capture;
	int step = m_sign * m_arwidth;
	bool isPromotion = m_squares[sourceSquare - step * 2].isWall();
	
	// One square ahead
	targetSquare = sourceSquare - step;
	capture = m_squares[targetSquare];
	if (capture.isEmpty()) {
		if (isPromotion)
			addPromotions(sourceSquare, targetSquare, moves);
		else {
			moves.push_back(Move(sourceSquare, targetSquare));
			
			// Two squares ahead
			if (m_squares[sourceSquare + step * 2].isWall()) {
				targetSquare -= step;
				capture = m_squares[targetSquare];
				if (capture.isEmpty())
					moves.push_back(Move(sourceSquare, targetSquare));
			}
		}
	}
	
	// Captures, including en-passant moves
	for (int i = -1; i <= 1; i += 2) {
		targetSquare = sourceSquare - step + i;
		capture = m_squares[targetSquare];
		if (capture.side() == !m_side
		||  targetSquare == m_enpassantSquare) {
			if (isPromotion)
				addPromotions(sourceSquare, targetSquare, moves);
			else
				moves.push_back(Move(sourceSquare, targetSquare));
		}
	}
}

bool Board::canCastle(int castlingSide) const
{
	int rookSq = m_castlingRights.rookSquare[m_side][castlingSide];
	if (rookSq == 0)
		return false;
	
	int kingSq = m_kingSquare[m_side];
	int target = m_castleTarget[m_side][castlingSide];
	int left;
	int right;
	int rtarget;
	
	// Find all the squares involved in the castling
	if (castlingSide == QueenSide) {
		rtarget = target + 1;
		
		if (target < rookSq)
			left = target;
		else
			left = rookSq;
		
		if (rtarget > kingSq)
			right = rtarget;
		else
			right = kingSq;
	} else {
		rtarget = target - 1;
		
		if (rtarget < kingSq)
			left = rtarget;
		else
			left = kingSq;
		
		if (target > rookSq)
			right = target;
		else
			right = rookSq;
	}
	
	// Make sure that the smallest back rank interval containing the king,
	// the castling rook, and their destination squares contains no pieces
	// other than the king and the castling rook.
	for (int i = left; i <= right; i++) {
		if (i != kingSq && i != rookSq && !m_squares[i].isEmpty())
			return false;
	}

	return true;
}

void Board::generateCastlingMoves(QVector<Chess::Move>& moves) const
{
	int source = m_kingSquare[m_side];
	for (int i = QueenSide; i <= KingSide; i++) {
		if (canCastle(i)) {
			int target = m_castleTarget[m_side][i];
			moves.push_back(Move(source, target, Piece::NoPiece, i));
		}
	}
}

void Board::generateHoppingMoves(int sourceSquare,
                                 const QVector<int>& offsets,
                                 QVector<Chess::Move>& moves) const
{
	foreach (int i, offsets)
	{
		int targetSquare = sourceSquare + i;
		Piece capture = m_squares[targetSquare];
		if (capture.isEmpty() || capture.side() == !m_side)
			moves.push_back(Move(sourceSquare, targetSquare));
	}
}

void Board::generateSlidingMoves(int sourceSquare,
                                 const QVector<int>& offsets,
                                 QVector<Chess::Move>& moves) const
{
	foreach (int i, offsets)
	{
		int targetSquare = sourceSquare + i;
		Piece capture;
		while (!(capture = m_squares[targetSquare]).isWall()
		&&      capture.side() != m_side) {
			moves.push_back(Move(sourceSquare, targetSquare));
			if (!capture.isEmpty())
				break;
			targetSquare += i;
		}
	}
}
