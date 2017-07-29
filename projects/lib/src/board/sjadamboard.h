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

#include <memory>
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
		SjadamBoard(const SjadamBoard*);

		// Inherited from WesternBoard
		virtual Board* copy() const override;
		virtual Result result() override;
		virtual QString variant() const override;
		virtual bool variantHasDrops() const override;
		virtual QString defaultFenString() const override;
		virtual bool vIsLegalMove(const Move& move) override;
		
		static Move toNormalMove(const Move& move);
		

	protected:

		// Inherited from WesternBoard
		virtual QString lanMoveString(const Move& move) override;
		virtual QString sanMoveString(const Move& move) override;
		
		virtual Move moveFromLanString(const QString& str) override;
		virtual Move moveFromSanString(const QString& str) override;
		
		virtual void vMakeMove(const Move& move,
				       BoardTransition* transition) override;
		virtual void vUndoMove(const Move& move) override;
		virtual void generateMovesForPiece(QVarLengthArray<Move>& moves,
						   int pieceType,
						   int square) const override;

	private:
		// std::unique_ptr<SjadamBoard> m_oldBoard;
		QVector<Move> m_moveHistory;
		int m_halfMoveClock = 0;
		int m_moveClock = 0;
};

} // namespace Chess
#endif // SJADAMBOARD_H
