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

#ifndef SJADAMBOARD_H
#define SJADAMBOARD_H

#include "westernboard.h"

namespace Chess {

/*!
 * \brief A board for Sjadam chess
 *
 */
class LIB_EXPORT SjadamBoard : public WesternBoard
{
	public:
		/*! Creates a new SjadamBoard object. */
		SjadamBoard();

		// Inherited from WesternBoard
		virtual Board* copy() const;
		virtual Result result();
		virtual QString variant() const;
		virtual bool variantHasDrops() const;
		virtual QString defaultFenString() const;
		//virtual bool vIsLegalMove(const Move& move);
		/*!
		 * Converts a Move into a string.
		 *
		 * \note The board must be in a position where \a move can be made.
		 * \sa moveFromString()
		 */
		QString moveString(const Move& move, MoveNotation notation);
		/*!
		 * Converts a move string into a Move.
		 *
		 * \note Returns a null move if \a move is illegal.
		 * \note Notation is automatically detected, and can be anything
		 * that's specified in MoveNotation.
		 * \sa moveString()
		 */
		Move moveFromString(const QString& str);

	protected:

		// Inherited from WesternBoard
		/*
		virtual bool kingsCountAssertion(int whiteKings,
						 int blackKings) const;
		virtual bool inCheck(Side side, int square = 0) const;
		*/
		virtual QString sanMoveString(const Move& move);
		virtual Move moveFromSanString(const QString& str);
		virtual void vMakeMove(const Move& move,
				       BoardTransition* transition);
		virtual void vUndoMove(const Move& move);
		virtual void generateMovesForPiece(QVarLengthArray<Move>& moves,
						   int pieceType,
						   int square) const;

	private:

};

} // namespace Chess
#endif // SJADAMBOARD_H
