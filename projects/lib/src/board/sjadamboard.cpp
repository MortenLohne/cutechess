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

using namespace std;

#include <iostream>
#include "move.h"
#include "sjadamboard.h"
#include "westernzobrist.h"
#include "boardtransition.h"

namespace Chess {

  /*! Notation for move strings. */
  enum MoveNotation
    {
      StandardAlgebraic,	//!< Standard Algebraic notation (SAN).
      LongAlgebraic		//!< Long Algebraic/Coordinate notation.
    };
  
  SjadamBoard::SjadamBoard()
    : WesternBoard(new WesternZobrist())
  {

  }

  Board* SjadamBoard::copy() const
  {
    return new SjadamBoard(*this);
  }
  
  Result SjadamBoard::result()
  {

	// It's checkmate if the either side's king has been captured
	int numKings = 0;
	int numPieces = 0;
	Side kingColor;
	for (int file = 0; file < height(); file++) {
	  for (int rank = 0; rank < width(); rank++) {
	    Square square = Chess::Square(file, rank);
	    const Piece piece = pieceAt(square);
	    switch (piece.type())
	      {
	      case King:
		numKings += 1;
		numPieces += 1;
		kingColor = piece.side();
		break;
	      case Piece::NoPiece:
		break;
	      default:
		numPieces += 1;
		break;
	      }
	    
	  }
	}
	if (numKings < 2) {
	  return Result(Result::Win, kingColor, tr("%1 mates").arg(kingColor.toString()));
	}
	// TODO: This will adjucate a draw even if either king can capture the other on the next move
	else if (numPieces == 2) {
	  return Result(Result::Draw, Side::NoSide, tr("Draw by stalemate"));
	}
	else {
	  return Result();
	}
  }

  QString SjadamBoard::variant() const
  {
    return "sjadam";
  }

  bool SjadamBoard::variantHasDrops() const
  {
    return false;
  }

  QString SjadamBoard::defaultFenString() const
  {
    return "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  }

  QString SjadamBoard::sanMoveString(const Move& move)
  {
    QString str = QString();
    if (move.sourceSquare() == move.sjadamSquare()) {
      str.append("-");
    }
    else {
      str.append(Board::squareString(move.sourceSquare()));
    }
    str.append(Board::squareString(move.sjadamSquare()));
    if (move.targetSquare() == move.sjadamSquare()) {
      str.append("-");
    }
    else {
      str.append(Board::squareString(move.targetSquare()));
    }
    return str;
  }

  QString SjadamBoard::moveString(const Move& move, MoveNotation notation) {
    return sanMoveString(move);
  }
  
  Move SjadamBoard::moveFromSanString(const QString& str)
  {
    qDebug("Parsing sjadam move string " + str.toLatin1());
    if (str[0] == '-') {
      if (str.length() == 6) {
	return WesternBoard::moveFromSanString(str.right(5));
      }
      else {
	return WesternBoard::moveFromSanString(str.right(4));
      }
    }
    else if (str[5] == '-') {
      int from = Board::squareIndex(str.midRef(0, 2).toString());
      int to = Board::squareIndex(str.midRef(2, 2).toString());
      return Move(from, to, 0, to);
    }
    else {
      // TODO: Parse promotions correctly
      int fromSquare = Board::squareIndex(str.midRef(0, 2).toString());
      int sjadamSquare = Board::squareIndex(str.midRef(2, 2).toString());
      int toSquare = Board::squareIndex(str.midRef(4, 2).toString());
      return Move(fromSquare, sjadamSquare, 0, toSquare);
    }
  }

  Move SjadamBoard::moveFromString(const QString& str) {
    return moveFromSanString(str);
  }

  void SjadamBoard::vMakeMove(const Move& move, BoardTransition* transition)
  {
    qDebug("Doing sjadam move " + sanMoveString(move).toLatin1());
    auto fromSquare = move.sourceSquare();
    auto sjadamSquare = move.sjadamSquare();
    auto toSquare = move.targetSquare();
    // TODO: Does not correctly revoke castling or en passant rights
    
    if (fromSquare != sjadamSquare) {
      setSquare(sjadamSquare, pieceAt(fromSquare));
      setSquare(fromSquare, Piece());
    }
    // TODO: Does not correctly increment half move counters 
    if (sjadamSquare != toSquare) {
      return WesternBoard::vMakeMove(move, transition);
    }
    // TODO: Does not promote pieces on back rank
    qDebug("Finished doing sjadam move");
  }

  void SjadamBoard::vUndoMove(const Move& move)
  {
    // TODO: Does not correctly restore castling rights
    qDebug("Undoing sjadam move " + sanMoveString(move).toLatin1());
    auto fromSquare = move.sourceSquare();
    auto sjadamSquare = move.sjadamSquare();
    auto toSquare = move.targetSquare();

    if (fromSquare != sjadamSquare) {
      setSquare(fromSquare, pieceAt(sjadamSquare));
      setSquare(sjadamSquare, Piece());
    }
    // Does not correctly increment half move counters 
    if (sjadamSquare != toSquare) {
      WesternBoard::vUndoMove(move);
    }
    // TODO: May not un-promote pieces on back rank
    qDebug("FInished undoing sjadam move");
  }

  // TODO: Implement correctly
  bool vIsLegalMove(const Move& move) {
    return true;
  }
  
  // TODO: Implement correctly
  void SjadamBoard::generateMovesForPiece(QVarLengthArray<Move>& moves,
					  int pieceType,
					  int square) const
  {
    WesternBoard::generateMovesForPiece(moves, pieceType, square);
  }

} // namespace Chess
