/*
 *
 *  Copyright ( c ) 2011
 *  name : mhogo mchungu
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  ( at your option ) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "openpartition.h"

#define ALL_PARTITIONS 1
#define NON_SYSTEM_PARTITIONS 3

openpartition::openpartition( QWidget * parent ) :
	QDialog( parent )
{
	m_ui = new Ui::PartitionView() ;
	m_ui->setupUi( this );
	this->setFixedSize( this->size() );
	this->setWindowFlags( Qt::Window | Qt::Dialog );
	this->setFont( parent->font() );

	connect( m_ui->tableWidget,SIGNAL( itemDoubleClicked( QTableWidgetItem * ) ),this,SLOT( tableEntryDoubleClicked( QTableWidgetItem * ) ) ) ;
	connect( m_ui->tableWidget,SIGNAL( currentItemChanged( QTableWidgetItem *,QTableWidgetItem * ) ),this,
		 SLOT( currentItemChanged( QTableWidgetItem *,QTableWidgetItem * ) ) ) ;
	connect( m_ui->pbHelp,SIGNAL( clicked() ),this,SLOT( pbHelp() ) );
	connect( m_ui->pbUUID,SIGNAL( clicked() ),this,SLOT( pbUUID() ) ) ;
	connect( m_ui->pbCancel,SIGNAL( clicked() ),this,SLOT( pbCancel() ) );

	m_action = new QAction( this ) ;
	QList<QKeySequence> keys ;
	keys.append( Qt::Key_Enter );
	keys.append( Qt::Key_Return );
	m_action->setShortcuts( keys ) ;

	connect( m_action,SIGNAL( triggered() ),this,SLOT( EnterKeyPressed() ) );

	this->addAction( m_action );

	QTableWidget * tw = m_ui->tableWidget ;
	tw->setColumnWidth( 0,90 );
	tw->setColumnWidth( 1,90 );
	tw->setColumnWidth( 2,110 );
	tw->setColumnWidth( 3,90 );
	tw->setColumnWidth( 4,309 );

	for( int i = 0 ; i < 5 ; i++ )
		tw->horizontalHeaderItem( i )->setFont( this->font() );

	tw->horizontalHeader()->setVisible( true );

	m_ui->checkBoxUUID->setVisible( false );

	m_diableNonLUKS = false ;
	//m_ui->pbUUID->setVisible( false );
}

void openpartition::pbHelp()
{
	DialogMsg msg( this ) ;

	QString m ;

	if( m_option == 2 ){
		m = tr( "a list of all partitions on this system are displayed here.\nDouble click an entry to use it" ) ;
	}else{
		if( getuid() != 0 ) {
			m = tr( "you are not root user and hence only non system partition are displayed on this list.\
\nPlease read documentation for more information on system/non system partitions.\nDouble click an entry to use it" ) ;
		}else{
			m = tr( "you are a root user and all partitions are displayed.\nDouble click an entry to use it" )	;
		}
	}
	msg.ShowUIOK( tr( "info" ),m );
}

void openpartition::pbUUID()
{
	if( m_ui->pbUUID->isFlat() )
		m_ui->pbUUID->setFlat( false );
	else
		m_ui->pbUUID->setFlat( true );
}

void openpartition::pbCancel()
{
	this->HideUI();
}

void openpartition::EnterKeyPressed()
{
	QTableWidget *tw = m_ui->tableWidget ;
	QTableWidgetItem *it = tw->currentItem() ;
	if( it == NULL )
		return ;
	tableEntryDoubleClicked( tw->item( it->row(),0 ) );
}

void openpartition::currentItemChanged( QTableWidgetItem * current, QTableWidgetItem * previous )
{
	if( current != NULL )
		HighlightRow( current->row(),true ) ;
	if( previous != NULL )
		if( current != NULL )
			if( previous->row() != current->row() )
				HighlightRow( previous->row(),false ) ;
}

void openpartition::HighlightRow( int r, bool b )
{
	for( int i = 0 ; i < 5 ; i++ )
		m_ui->tableWidget->item( r,i )->setSelected( b );
	if( b==true )
		m_ui->tableWidget->setCurrentCell( r,4 );
}

void openpartition::ShowNonSystemPartitions()
{
	m_option = 1 ;
	partitionList( tr( "select a partition to create an encrypted volume in" )," -N" );
}

void openpartition::ShowAllPartitions()
{
	m_option = 2 ;
	partitionList( tr( "select an encrypted partition to open" )," -A" );
}

void openpartition::ShowPartitionList( QString x,QString y )
{
	partitionList( x,y );
}

void openpartition::allowLUKSOnly()
{
	m_diableNonLUKS = true ;
	this->ShowAllPartitions();
}

void openpartition::partitionList( QString title,QString type )
{
	this->setWindowTitle( title );

	while ( m_ui->tableWidget->rowCount() > 0 )
		m_ui->tableWidget->removeRow( 0 );

	partitionproperties * op = new partitionproperties( type );

	connect( op,SIGNAL( finished() ),this,SLOT( partitionpropertiesThreadFinished() ) );
	connect( op,SIGNAL( partitionProperties( QStringList ) ),this,SLOT( partitionProperties( QStringList ) ) );
	m_ui->tableWidget->setEnabled( false );
	op->start();
	this->show();
}

void openpartition::partitionProperties( QStringList entry )
{
	QTableWidgetItem * item ;
	int row = m_ui->tableWidget->rowCount() ;
	m_ui->tableWidget->insertRow( row );
	int h = m_ui->tableWidget->rowHeight( 0 ) ;
	for( int i = 0 ; i < 5 ; i++ ){
		item = new QTableWidgetItem( entry.at( i ) ) ;
		item->setTextAlignment( Qt::AlignCenter );
		m_ui->tableWidget->setItem( row,i,item );
		m_ui->tableWidget->setRowHeight( row,h );
	}
	m_ui->tableWidget->setCurrentCell( row,4 );
}

void openpartition::partitionpropertiesThreadFinished()
{
	m_ui->tableWidget->setEnabled( true );
	m_ui->tableWidget->setFocus();
}

void openpartition::HideUI()
{
	this->hide();
	emit HideUISignal();
}

void openpartition::tableEntryDoubleClicked( QTableWidgetItem * item )
{
	QString dev ;
	QTableWidget * tw = m_ui->tableWidget ;

	if( m_diableNonLUKS ){
		if( tw->item( item->row(),3 )->text() != QString( "crypto_LUKS") ){
			DialogMsg m( this ) ;

			return m.ShowUIOK( tr( "ERROR" ),tr( "only cryto_LUKS volumes can be selected" ) ) ;
		}else{
			m_ui->pbUUID->setFlat( true );
		}

	}
	//if( m_ui->checkBoxUUID->isChecked() == true )
	if( m_ui->pbUUID->isFlat() )
		dev = QString( "UUID=\"" ) + tw->item( item->row(),4 )->text() + QString( "\"" ) ;
	else
		dev = tw->item( item->row(),0 )->text() ;

	emit clickedPartition( dev ) ;
	HideUI();
}

void openpartition::closeEvent( QCloseEvent * e )
{
	e->ignore();
	HideUI();
}

openpartition::~openpartition()
{
	delete m_ui ;
	delete m_action ;
}
