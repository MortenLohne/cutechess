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

#include <utility>
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

  Board* SjadamBoard::copy() const {
    qWarning("Tried to copy sjadam board");
    return 0;
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

  QString SjadamBoard::lanMoveString(const Move& move) 
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
    Q_ASSERT(str.length() >= 5);
    return str;
  }

  QString SjadamBoard::sanMoveString(const Move& move) {
    return lanMoveString(move);
  }
  
  Move SjadamBoard::moveFromLanString(const QString& str)
  {
    if (str[0] == '-') {
      if (str.length() == 6) {
	return WesternBoard::moveFromLanString(str.right(5));
      }
      else {
	return WesternBoard::moveFromLanString(str.right(4));
      }
    }
    else if (str[4] == '-') {
      int from = Board::squareIndex(str.midRef(0, 2).toString());
      int to = Board::squareIndex(str.midRef(2, 2).toString());
      return Move(from, to, 0, to);
    }
    else if (str.length() == 6) {
      int fromSquare = Board::squareIndex(str.midRef(0, 2).toString());
      int sjadamSquare = Board::squareIndex(str.midRef(2, 2).toString());
      int toSquare = Board::squareIndex(str.midRef(4, 2).toString());
      Move move = Move(fromSquare, toSquare, 0, sjadamSquare);
      return move;
    }
    else {
      qWarning("Failed to parse " + str.toLatin1());
      return Move();
    }
  }
  Move SjadamBoard::moveFromSanString(const QString& str) {
    return moveFromLanString(str);
  }


  void SjadamBoard::vMakeMove(const Move& move, BoardTransition* transition)
  {
    //qInfo("Making move " + lanMoveString(move).toLatin1() + " on " + fenString().toLatin1());
    m_moveHistory.push_back(move);
    int fromSquare = move.sourceSquare();
    int sjadamSquare = move.sjadamSquare();
    int toSquare = move.targetSquare();
    // TODO: Does not correctly revoke castling or en passant rights
    // First do sjadam jump, if any
    if (sjadamSquare != fromSquare) {
      //qInfo("Doing sjadam jump from " + squareString(fromSquare).toLatin1() + ", id=%d", fromSquare);
      setSquare(sjadamSquare, pieceAt(fromSquare));
      setSquare(fromSquare, Piece());
    }
    // TODO: Does not correctly increment half move counters
    // Do regular chess move, if any
    Move chessMove = SjadamBoard::toNormalMove(move);
    if (sjadamSquare != toSquare) {
      //qInfo("Doing regular chess move part");
      WesternBoard::vMakeMove(chessMove, transition);
    }
    // Changing sides is taken care of in board::makeMove()
    if (pieceAt(toSquare).type() != King &&
	((chessSquare(toSquare).rank() == 7 && m_side == Side::White)
	 || (chessSquare(toSquare).rank() == 0 && m_side == Side::Black))) {
      
      Piece piece = pieceAt(toSquare);
      Piece queen = Piece(piece.side(), Queen);
      setSquare(toSquare, queen);
      //qInfo("Promoting piece with move %s at board %s", qPrintable(lanMoveString(move)), qPrintable(fenString()));
    }
    //qInfo("Finished making move " + lanMoveString(move).toLatin1() + " on " + fenString().toLatin1());
  }

  Move SjadamBoard::toNormalMove(const Move& move) {
    return Move(move.sjadamSquare(), move.targetSquare(), move.promotion(), move.sjadamSquare());
  }

  
  
  void SjadamBoard::vUndoMove(const Move& move)
  {
    auto newBoard = SjadamBoard();
    newBoard.initialize();
    newBoard.setFenString(m_startFen);
    if (m_moveHistory.isEmpty() || move != m_moveHistory.last()) {
      qWarning("Undoing move that wasn't the last move");
    }
    if (!m_moveHistory.isEmpty()) {
      m_moveHistory.removeLast();
    }
    
    for (auto move : m_moveHistory) {
      newBoard.makeMove(move);
    }
    *this = newBoard;
    //qInfo("New board dimensions: %d, %d", width(), height());
    //qInfo("Finished undoing move " + lanMoveString(move).toLatin1());
  }

  // TODO: Implement correctly
  bool SjadamBoard::vIsLegalMove(const Move& move) {
    qInfo("Checking if " + lanMoveString(move).toLatin1() + " is legal.");
    return true;
  }

  bool SjadamBoard::vSetFenString(const QStringList& fen) {
    m_startFen = fenString();
    m_moveHistory.clear();
  }
  
  // TODO: Implement correctly
  void SjadamBoard::generateMovesForPiece(QVarLengthArray<Move>& moves,
					  int pieceType,
					  int square) const
  {
    WesternBoard::generateMovesForPiece(moves, pieceType, square);
  }

} // namespace Chess
